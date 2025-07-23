#!/bin/bash
# helloworld_runner.sh
# 这些变量在 bazel run 时可用！
echo "BUILD_WORKSPACE_DIRECTORY: $BUILD_WORKSPACE_DIRECTORY"
echo "BUILD_WORKING_DIRECTORY: $BUILD_WORKING_DIRECTORY"
cd $BUILD_WORKSPACE_DIRECTORY/example/helloworld
pwd
bazel build //cpp:greeter_client
bazel build //cpp:greeter_server