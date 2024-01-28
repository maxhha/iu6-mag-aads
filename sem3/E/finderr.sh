while read -r line; do
    result="$(printf "10\n$line\n" | ./a.out)"
    [[ $result != "1 2 3 4 5 6 7 8 9 10" ]] && echo $line \| $result
    printf "."
done < r_s3.txt
