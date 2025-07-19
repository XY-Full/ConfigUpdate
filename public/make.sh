#!/bin/bash 
path=$(pwd)
export LD_LIBRARY_PATH=$(pwd)/../third/protobuf/lib:$LD_LIBRARY_PATH

mkdir proto_files -p

protoc_path=$path/../third/protobuf/bin

# 仅在 .proto 被修改时重新编译
compile_proto_if_needed() {
    src_dir="$1"
    out_dir="$2"
    # shift 2是让"$@"从第三个参数开始，方便遍历
    shift 2
    for proto_file in "$@"; do
        proto_path="$src_dir/$proto_file"
        base_name=$(basename "$proto_file" .proto)
        out_cc="$out_dir/${base_name}.pb.cc"
        out_h="$out_dir/${base_name}.pb.h"

        # -nt 是检测文件是否比另一个文件更（gèng）新 翻译为英文叫做 newer than
        if [[ ! -f "$out_cc" || "$proto_path" -nt "$out_cc" ]]; then
            echo -e "${BLUE}Recompiling $proto_file ...${NC}"
            cd $src_dir && $protoc_path/protoc --proto_path=. --proto_path=$path/proto --cpp_out=$out_dir $proto_file
        # else
        #     echo -e "${YELLOW}Up-to-date: $proto_file${NC}"
        fi
    done
}

cd "$path/proto"
proto_files=(*.proto)
compile_proto_if_needed "$path/proto" "$path/proto_files" "${proto_files[@]}"
