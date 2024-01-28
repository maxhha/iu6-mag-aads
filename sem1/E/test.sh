gcc *.c && (
    for file in $( ls in_* )
    do
        echo -e "\n$file:"
        cat $file
        cat $file |
        ./a.out
    done
)
