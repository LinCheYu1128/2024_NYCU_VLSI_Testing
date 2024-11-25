#!/bin/bash

# Assign arguments to variables
command_key=$1
circuit_name=$2
pattern_num=$3
backtrack=$2

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
    ass5_a)
        ./atpg -output ../input/${circuit_name}.input ../circuits/iscas85/${circuit_name}.bench
        ./atpg -check_point -output ../input/${circuit_name}.cp_input ../circuits/iscas85/${circuit_name}.bench
        ;;
    ass5_b)
        ./atpg -fsim -input ../input/${circuit_name}.input ../circuits/iscas85/${circuit_name}.bench
        ;;
    ass5_c)
        ./atpg -bridging_fsim -input ../input/${circuit_name}.input ../circuits/iscas85/${circuit_name}.bench
        ;;
    ass6_a)
        ./atpg -bt ${backtrack} -output ../input/b17.input ../circuits/b17.bench 
        ;;
    ass6_b1)
        ./atpg -bt 1 -check_point -output ../input/b17.cp_input ../circuits/b17.bench
        ./atpg -fsim -input ../input/b17.cp_input ../circuits/b17.bench
        ;;
    ass6_b2)
        ./atpg -bt 1 -output ../input/b17.input ../circuits/b17.bench
        ./atpg -fsim -input ../input/b17.input ../circuits/b17.bench
        ;;
    ass6_c)
        ./atpg -c17_proc -output ../input/c17.input ../circuits/iscas85/c17.bench
        ;;
    ass6_d)
        if [[ ${circuit_name} == "b17" ]]; then
            ./atpg -bt 10 -random_pattern -output ../input/${circuit_name}.input ../circuits/${circuit_name}.bench
        else
            ./atpg -bt 10 -random_pattern -output ../input/${circuit_name}.input ../circuits/iscas89_com/${circuit_name}.bench
        fi
        ;;
    ass6_e)
        # ./atpg -bt 1 -output ../input/${circuit_name}.binput ../circuits/iscas85/${circuit_name}.bench
        ./atpg -bridging_atpg -output ../input/${circuit_name}.binput ../circuits/iscas85/${circuit_name}.bench
        ./atpg -bridging_fsim -input ../input/${circuit_name}.input ../circuits/iscas85/${circuit_name}.bench
        ;;
    *)
        echo "Unknown command key: $command_key"
        exit 1
        ;;
esac