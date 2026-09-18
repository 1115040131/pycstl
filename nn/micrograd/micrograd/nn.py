import random
from micrograd.engine import Value, Number
from typing import List, Sequence, Union, cast

# 层的输入可能是原始数值, 也可能是上一层输出的 Value
Input = Sequence[Union[Value, Number]]


class Module:
    def zero_grad(self):
        for p in self.parameters():
            p.grad = 0

    def parameters(self) -> List[Value]:
        return []


class Neuron(Module):
    def __init__(self, nin: int, nonlin: bool = True):
        self.w: List[Value] = [Value(random.uniform(-1, 1))for _ in range(nin)]
        self.b: Value = Value(0)
        self.nonlin = nonlin

    def __call__(self, x: Input) -> Value:
        # w * x + b
        act = sum((wi * xi for wi, xi in zip(self.w, x)), self.b)
        return act.relu() if self.nonlin else act

    def parameters(self) -> List[Value]:
        return self.w + [self.b]

    def __repr__(self):
        return f"{'ReLU' if self.nonlin else 'Linear'}Neuron({len(self.w)})"


class Layer(Module):
    def __init__(self, nin: int, nout: int, **kwargs):
        self.neurons = [Neuron(nin, **kwargs) for _ in range(nout)]

    def __call__(self, x: Input) -> Union[Value, List[Value]]:
        outs = [n(x) for n in self.neurons]
        return outs[0] if len(outs) == 1 else outs

    def parameters(self) -> List[Value]:
        return [p for neuron in self.neurons for p in neuron.parameters()]

    def __repr__(self):
        return f"Layer of [{', '.join(str(n) for n in self.neurons)}]"


class MLP(Module):
    def __init__(self, nin: int, nouts: List[int]):
        sz = [nin] + nouts
        self.layers = [Layer(sz[i], sz[i+1], nonlin = i != len(nouts) - 1) for i in range(len(nouts))]

    def __call__(self, x: Input) -> Union[Value, List[Value]]:
        # 中间层输出恒为 List[Value], 仅最后一层可能返回单个 Value
        out: Union[Value, List[Value]] = cast(List[Value], x)
        for layer in self.layers:
            out = layer(cast(List[Value], out))
        return out

    def parameters(self) -> List[Value]:
        return [p for layer in self.layers for p in layer.parameters()]

    def __repr__(self):
        return f"MLP of [{', '.join(str(layer) for layer in self.layers)}]"
