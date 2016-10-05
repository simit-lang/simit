
# small script to launch several instances of lulesh-simit and get time results

version=$1
launchs=$2
#size=$3
iters=$3

cd build

for size in {150..150}
do
echo "Version : "$version" launchs : "$launchs" size : "$size" iters : "$iters

for i in $(eval echo "{1..$launchs}")
   do
	./lulesh-$version -s $size -i $iters -r 1 > tmpTime
    grep Elapsed tmpTime | cut -d "=" -f 2 | cut -f 1 -d "("
done
rm tmpTime
done
cd -

