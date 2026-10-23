CLEAR_RUNS=${CLEAR_RUNS:-false}

cd results
rm -f *.log
rm -f *.tmp
rm -f *.txt

if [ "$CLEAR_RUNS" = "true" ]; then
    rm -f *.out
fi