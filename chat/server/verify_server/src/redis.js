const config_module = require('./config');
const Redis = require('ioredis');

// 创建 redis 客户端
const RedisClient = new Redis({
    host: config_module.redis_host,
    port: config_module.redis_port,
    password: config_module.redis_password
});

// 监听错误信息
RedisClient.on("error", function (err) {
    console.log("RedisClient connect error");
    RedisClient.quit();
});

// 根据 key 获取 value
async function GetRedis(key) {
    try {
        const result = await RedisClient.get(key);
        if (result === null) {
            console.log('result:' + '<' + result + '>', 'This key cannot be found...');
            return null;
        }
        console.log('result:' + '<' + result + '>', 'Get key successfully...');
        return result;
    } catch (err) {
        console.log('GetRedis err:', err);
        return null;
    }
}

// 根据 key 查询是否存在
async function QueryRedis(key) {
    try {
        const result = await RedisClient.exists(key);
        if (result === 0) {
            console.log('result:' + '<' + result + '>', 'This key is null...');
            return null;
        }
        console.log('result:' + '<' + result + '>', 'This key exists...');
        return result;
    } catch (error) {
        console.log('QueryRedis err:', error);
        return null;
    }
}

// 设置 key-value 和过期时间
async function SetRedisExpire(key, value, expire) {
    try {
        await RedisClient.set(key, value);
        await RedisClient.expire(key, expire);
        return true;
    } catch (error) {
        console.log('SetRedisExpire err:', error);
        return false;
    }
}

// 退出函数
function Quit() {
    RedisClient.quit();
}

module.exports = { GetRedis, QueryRedis, SetRedisExpire, Quit };