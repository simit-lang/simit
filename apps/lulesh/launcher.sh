
# small script to launch several instances of lulesh-simit and get time results

launchs=$1
for i in $(eval echo "{1..$launchs}")
   do
    ./build/lulesh-simit > tmpTime
    grep Elapsed tmpTime | cut -d "=" -f 2 | cut -f 1 -d "("
done
rm tmpTime

