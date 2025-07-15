# 开发者指南

## 环境搭建

### bazel

MRPC 采用了 bazel 8.3.1 进行构建，在用户目录中执行下列命令即可完成安装

```shell
wget https://github.com/bazelbuild/bazel/releases/download/8.3.1/bazel-8.3.1-installer-linux-x86_64.sh
chmod +x bazel-8.3.1-installer-linux-x86_64.sh
./bazel-8.3.1-installer-linux-x86_64.sh --user

cat <<'EOF' >> ~/.bashrc

export PATH="$PATH:$HOME/bin"
source /home/what/.bazel/bin/bazel-complete.bash
EOF

source .bashrc
rm bazel-8.3.1-installer-linux-x86_64.sh
```

vscode有个bazel插件，使用需要安装buildifier，需要提前安装go
```shell
go install github.com/bazelbuild/buildtools/buildifier@latest
```
由于这是第三方库，因此会安装到 `~/go/bin` 目录下，经过测试需要在vscode的Bazel插件配置中 `Buildifier executable` 项设置可执行文件的准确路径: `~/go/bin/buildifier`。

### clang

由于clangd的存在，我们默认使用clang进行编译

> 虽然 cgo 能用 clang 编译，但 vscode 的有关插件并不能很好的识别C接口，因此在写 go 时建议切换到 gcc 编译：`bazel build --action_env CC=gcc`

执行下列命令即可安装 clang-19

```shell
wget https://apt.llvm.org/llvm.sh
chmod +x llvm.sh
sudo apt install lsb-release software-properties-common gnupg
sudo ./llvm.sh 19
rm llvm.sh

sudo update-alternatives --install /usr/bin/clangd clangd /usr/bin/clangd-19 100
sudo update-alternatives --install /usr/bin/clang clang /usr/bin/clang-19 100
sudo update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-19 100
```
### go
执行下面的命令即可安装 go 1.24.3，并配置代理
```shell
wget https://go.dev/dl/go1.24.3.linux-amd64.tar.gz
sudo rm -rf /usr/local/go && sudo tar -C /usr/local -xzf go1.24.3.linux-amd64.tar.gz

echo -e "\nexport PATH=\$PATH:/usr/local/go/bin" >> ~/.bashrc
source ~/.bashrc

go env -w GO111MODULE=on
go env -w GOPROXY=https://goproxy.cn,direct
```

## 编译
下面以 `example/helloworld` 为例来说明如何编译，下列指令需要在项目主目录执行

### 编译 cpp 客户端和服务端
```shell
bazel build //example/helloworld/cpp:greeter_client //example/helloworld/cpp:greeter_server
```
生成的可执行文件在 `./bazel-bin/example/helloworld/cpp/` 中

### 编译 python 客户端
```shell
bazel build //example/helloworld/python:greeter_client
```
生成的可执行文件在 `./bazel-bin/example/helloworld/python/` 中

### 编译 go 客户端
```shell
bazel build //example/helloworld/go:greeter_client
```
生成的可执行文件在 `./bazel-bin/example/helloworld/go/greeter_client_/` 中


