"""
FineWeb-Edu dataset (for srs pretraining)
https://huggingface.co/datasets/HuggingFaceFW/fineweb-edu
Downloads and tokenizes the data and saves data shards to disk.
Run simply as:
$ python fineweb.py
Will save shards to the local directory "edu_fineweb10B".
"""

import os
os.environ.setdefault("HF_ENDPOINT", "https://hf-mirror.com") # 设置国内镜像, 用于列目录 (走普通 HTTP API, 没有 Xet 问题)
import multiprocessing as mp
import numpy as np
import requests
import tiktoken
from huggingface_hub import HfApi
from datasets import load_dataset # pip install datasets
from tqdm import tqdm # pip install tqdm

# ------------------------------------------
local_dir = "edu_fineweb10B"
remote_name = "sample-10BT"
shard_size = int(1e8) # 100M tokens per shard, total of 100 shards

# create the cache the local directory if it doesn't exist yet
DATA_CACHE_DIR = os.path.join(os.path.dirname(__file__), local_dir)
os.makedirs(DATA_CACHE_DIR, exist_ok=True)

MODELSCOPE_RESOLVE_URL = "https://www.modelscope.cn/datasets/HuggingFaceFW/fineweb-edu/resolve/master"


def download_shard(repo_path: str, dest_path: str) -> None:
    """从 ModelScope 镜像下载单个 parquet 分片, 支持断点续传。

    该仓库文件已迁移到 HF 的 Xet 存储, 实际字节托管在 us.aws.cdn.hf.co,
    国内网络下载会超时, 而 hf-mirror.com 目前代理不到这条路径, 所以改用
    ModelScope 上的同名镜像仓库 (文件目录结构与 HF 一致)。
    """
    if os.path.exists(dest_path):
        return
    tmp_path = dest_path + ".tmp"
    resume_from = os.path.getsize(tmp_path) if os.path.exists(tmp_path) else 0
    headers = {"Range": f"bytes={resume_from}-"} if resume_from else {}
    with requests.get(f"{MODELSCOPE_RESOLVE_URL}/{repo_path}", headers=headers, stream=True, timeout=30) as r:
        r.raise_for_status()
        total = resume_from + int(r.headers.get("content-length", 0))
        with open(tmp_path, "ab" if resume_from else "wb") as f, tqdm(
            total=total, initial=resume_from, unit="B", unit_scale=True, desc=os.path.basename(repo_path)
        ) as bar:
            for chunk in r.iter_content(chunk_size=1024 * 1024):
                f.write(chunk)
                bar.update(len(chunk))
    os.rename(tmp_path, dest_path)


# download the dataset
# 只列子集目录 (sample/10BT, 十几个文件, 单页返回), 不对整个仓库 data/ 目录做递归遍历
# (上万文件, 分页请求容易在国内镜像下穿透回 huggingface.co 导致连接被重置)
repo_dir = remote_name.replace('-', '/', 1) # "sample-10BT" -> "sample/10BT"
raw_dir = os.path.join(DATA_CACHE_DIR, "raw_parquet")
os.makedirs(raw_dir, exist_ok=True)

shards = HfApi().list_repo_tree("HuggingFaceFW/fineweb-edu", repo_type="dataset",
                                path_in_repo=repo_dir, recursive=False)
local_paths = []
for shard in sorted(shards, key=lambda s: s.path):
    if not shard.path.endswith(".parquet"):
        continue
    dest_path = os.path.join(raw_dir, os.path.basename(shard.path))
    download_shard(shard.path, dest_path)
    local_paths.append(dest_path)

fw = load_dataset("parquet", data_files=local_paths, split="train")

# init the tokenizer
enc = tiktoken.get_encoding("gpt2")
eot = enc._special_tokens['<|endoftext|>'] # end of text token
def tokenize(doc):
    # tokenizes a single document and returns a numpy array of uint16 tokens
    tokens = [eot] # the special <|endoftext|> token delimits all documents
    tokens.extend(enc.encode_ordinary(doc["text"]))
    tokens_np = np.array(tokens)
    assert (0 <= tokens_np).all() and (tokens_np < 2**16).all(), "token dictionary too large for uint16"
    tokens_np_uint16 = tokens_np.astype(np.uint16)
    return tokens_np_uint16

def write_datafile(filename, tokens_np):
    np.save(filename, tokens_np)

# tokenize all documents and write output shards, each of shard_size tokens (last shard has remainder)
nprocs = max(1, os.cpu_count()//2)
with mp.Pool(nprocs) as pool:
    shard_index = 0
    # preallocate buffer to hold current shard
    all_tokens_np = np.empty((shard_size,), dtype=np.uint16)
    token_count = 0
    progress_bar = None
    for tokens in pool.imap(tokenize, fw, chunksize=16):

        # is there enough space in the current shard for the new tokens?
        if token_count + len(tokens) < shard_size:
            # simply append tokens to current shard
            all_tokens_np[token_count:token_count+len(tokens)] = tokens
            token_count += len(tokens)
            # update progress bar
            if progress_bar is None:
                progress_bar = tqdm(total=shard_size, unit="tokens", desc=f"Shard {shard_index}")
            progress_bar.update(len(tokens))
        else:
            # write the current shard and start a new one
            split = "val" if shard_index == 0 else "train"
            filename = os.path.join(DATA_CACHE_DIR, f"edufineweb_{split}_{shard_index:06d}")
            # split the document into whatever fits in this shard; the remainder goes to next one
            remainder = shard_size - token_count
            progress_bar.update(remainder)
            all_tokens_np[token_count:token_count+remainder] = tokens[:remainder]
            write_datafile(filename, all_tokens_np)
            shard_index += 1
            progress_bar = None
            # populate the next shard with the leftovers of the current doc
            all_tokens_np[0:len(tokens)-remainder] = tokens[remainder:]
            token_count = len(tokens)-remainder

    # write any remaining tokens as the last shard
    if token_count != 0:
        split = "val" if shard_index == 0 else "train"
        filename = os.path.join(DATA_CACHE_DIR, f"edufineweb_{split}_{shard_index:06d}")
        write_datafile(filename, all_tokens_np[:token_count])
