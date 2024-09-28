#!/bin/bash

# Assign arguments to variables
command_key=$1
circuit_name=$2
pattern_num=$3

# Construct and run the command
case $command_key in
    ass2_1)
        /usr/bin/time -v ./atpg -logicsim -input ../input/c$circuit_name.input -output ../output/c$circuit_name.output ../circuits/iscas85/c$circuit_name.bench
        ;;
    ass2_2a)
        ./atpg -pattern -num $pattern_num -output ../input/c$circuit_name.input ../circuits/iscas85/c$circuit_name.bench   
        ;;
    ass2_2aU)
        ./atpg -pattern -num $pattern_num -unknown -output ../input/c$circuit_name.input ../circuits/iscas85/c$circuit_name.bench
        ;;
    ass2_2b)
        /usr/bin/time -v ./atpg -mod_logicsim -input ../input/c$circuit_name.input -output ../output/c${circuit_name}_m.output ../circuits/iscas85/c$circuit_name.bench
        ;;
    help)
        echo "For assignment 2:"
        echo "  Usage: ./run.sh ass2_1   <circuit_name>"
        echo "  Usage: ./run.sh ass2_2a  <circuit_name> <pattern_num>"
        echo "  Usage: ./run.sh ass2_2aU <circuit_name> <pattern_num>"
        echo "  Usage: ./run.sh ass2_2b  <circuit_name>"
        ;;
    *)
        echo "Unknown command key: $command_key"
        echo "Available keys: ass2_1, ass2_2a, ass2_2aU, ass2_2b, help"
        exit 1
        ;;
esac