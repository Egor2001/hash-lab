#!/bin/sh

if [ $# -ne 1 ] 
then
    echo "USAGE: $0 TL_SECONDS"
    exit
fi

echo 'LAUNCH linear std...'
./bin/main linear murmur3 bench/linear-std.txt $1
echo 'GENERATED bench/linear-std.txt'

echo 'LAUNCH linear sha256...'
./bin/main linear murmur3 bench/linear-sha256.txt $1
echo 'GENERATED bench/linear-sha256.txt'

echo 'LAUNCH linear md5...'
./bin/main linear murmur3 bench/linear-md5.txt $1
echo 'GENERATED bench/linear-md5.txt'

echo 'LAUNCH linear polynomial...'
./bin/main linear murmur3 bench/linear-polynomial.txt $1
echo 'GENERATED bench/linear-polynomial.txt'

echo 'LAUNCH linear tabulation...'
./bin/main linear murmur3 bench/linear-tabulation.txt $1
echo 'GENERATED bench/linear-tabulation.txt'
