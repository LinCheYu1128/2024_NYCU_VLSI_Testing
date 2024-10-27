#!/bin/bash

# Assign arguments to variables
command_key=$1
circuit_name=$2
pattern_num=$3

# Construct and run the command
case $command_key in
    ass2_1)
        /usr/bin/time -f "Average memory usage: %K \nMaximum memory usage: %M\n" ./atpg -logicsim -input ../input/${circuit_name}.input -output ../output/${circuit_name}.output ../circuits/iscas85/${circuit_name}.bench
        ;;
    ass2_2a)
        ./atpg -pattern -num ${pattern_num} -output ../input/${circuit_name}.input ../circuits/iscas85/${circuit_name}.bench   
        ;;
    ass2_2aU)
        ./atpg -pattern -num ${pattern_num} -unknown -output ../input/${circuit_name}.input ../circuits/iscas85/${circuit_name}.bench
        ;;
    ass2_2b)
        /usr/bin/time -f "Average memory usage: %K \nMaximum memory usage: %M\n" ./atpg -mod_logicsim -input ../input/${circuit_name}.input -output ../output/${circuit_name}_m.output ../circuits/iscas85/${circuit_name}.bench
        ;;
    comp)
        python3 ../compare.py ../output/${circuit_name}.output ../output/${circuit_name}_m.output
        ;;
    ass3)
        /usr/bin/time -f "Average memory usage: %K \nMaximum memory usage: %M\n" ./atpg -plogicsim -input ../input/${circuit_name}.input -output ../output/${circuit_name}_p.output ../circuits/iscas85/${circuit_name}.bench
        ;;
    ass3_c)
        ./atpg -simulator ../simulator/${circuit_name}.cc -input ../input/${circuit_name}.input ../circuits/iscas85/${circuit_name}.bench
        ;;
    ass4_a)
        ./atpg -check_point ../circuits/iscas85/${circuit_name}.bench
        ;;
    ass4_b)
        ./atpg -bridging -output ../output/${circuit_name}.bfault ../circuits/iscas85/${circuit_name}.bench
        ;;
    help)
        echo "For assignment 2:"
        echo "  Usage: ./run.sh ass2_1   <circuit_name>"
        echo "  Usage: ./run.sh ass2_2a  <circuit_name> <pattern_num>"
        echo "  Usage: ./run.sh ass2_2aU <circuit_name> <pattern_num>"
        echo "  Usage: ./run.sh ass2_2b  <circuit_name>"
        echo "  Usage: ./run.sh comp     <circuit_name>"
        ;;
    *)
        echo "Unknown command key: $command_key"
        echo "Available keys: ass2_1, ass2_2a, ass2_2aU, ass2_2b, help"
        exit 1
        ;;
esac