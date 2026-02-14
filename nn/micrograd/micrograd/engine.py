from __future__ import annotations  # 允许在类定义内部使用类名作为类型

import math
from typing import Callable, List, Set, Tuple, Union

Number = Union[int, float]


class Value:
    def __init__(self, data: Number, _children: Tuple[Value, ...] = (), _op: str = '', label: str = ''):
        self.data: float = float(data)
        self.grad: float = 0.0
        self._backward: Callable[[], None] = lambda: None
        self._prev: Set[Value] = set(_children)  # 这里需要 set，所以 Value 必须可哈希
        self._op: str = _op
        self.label: str = label

    def __repr__(self) -> str:
        return f"Value(data={self.data}, grad(self.grad))"

    # --- 算术运算 ---

    def __add__(self, other: Union[Value, Number]) -> Value:  # self + other
        other = other if isinstance(other, Value) else Value(other)
        out = Value(self.data + other.data, (self, other), '+')

        def _backward():
            self.grad += 1.0 * out.grad
            other.grad += 1.0 * out.grad
        out._backward = _backward

        return out

    def __neg__(self):  # -self
        return self * -1

    def __sub__(self, other: Union[Value, Number]) -> Value:  # self - other
        return self + (-other)

    def __mul__(self, other: Union[Value, Number]) -> Value:  # self * other
        other = other if isinstance(other, Value) else Value(other)
        out = Value(self.data * other.data, (self, other), '*')

        def _backward():
            self.grad += other.data * out.grad
            other.grad += self.data * out.grad
        out._backward = _backward

        return out

    def __truediv__(self, other: Union[Value, Number]) -> Value:  # self / other
        return self * (other ** -1)

    def __pow__(self, other: Number) -> Value:  # self ** other
        assert isinstance(other, (int, float)
                          ), "only supporting int/float powers for now"
        out = Value(self.data ** other, (self, ), f'**{other}')

        def _backward():
            self.grad += (other * self.data**(other - 1)) * out.grad
        out._backward = _backward

        return out

    # --- 激活函数 ---

    def tanh(self) -> Value:  # tanh(self)
        x = self.data
        t = (math.exp(2 * x) - 1) / (math.exp(2 * x) + 1)
        out = Value(t, (self,), 'tanh')

        def _backward():
            self.grad += (1 - t**2) * out.grad
        out._backward = _backward

        return out

    def exp(self) -> Value:  # exp(self)
        x = self.data
        out = Value(math.exp(x), (self,), 'exp')

        def _backward():
            self.grad += out.data * out.grad
        out._backward = _backward

        return out

    def relu(self) -> Value:  # relu(self)
        out = Value(0 if self.data < 0 else self.data, (self,), 'ReLU')

        def _backward():
            self.grad += (out.data > 0) * out.grad
        out._backward = _backward

        return out

    # 反向算术支持 (例如: 2 * a)
    def __radd__(self, other: Number) -> Value:  # other + self
        return self + other

    def __rsub__(self, other: Number) -> Value:  # other - self
        return other+(-self)

    def __rmul__(self, other: Number) -> Value:  # other * self
        return self * other

    def __rtruediv__(self, other: Number) -> Value:  # other / self
        return other * (self ** -1)

    # --- 反向传播入口 ---

    def backward(self):
        topo: List[Value] = []
        visited: Set[Value] = set()

        def build_topo(v: Value):
            if v not in visited:
                visited.add(v)
                for child in v._prev:
                    build_topo(child)
                topo.append(v)
        build_topo(self)

        self.grad = 1.0
        for node in reversed(topo):
            node._backward()
