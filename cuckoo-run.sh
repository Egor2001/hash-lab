#!/bin/sh

if [ $# -ne 1 ] 
then
    echo "USAGE: $0 TL_SECONDS"
    exit
fi

echo 'LAUNCH cuckoo std...'
./bin/main cuckoo murmur3 bench/cuckoo-std.txt $1
echo 'GENERATED bench/cuckoo-std.txt'

echo 'LAUNCH cuckoo sha256...'
./bin/main cuckoo murmur3 bench/cuckoo-sha256.txt $1
echo 'GENERATED bench/cuckoo-sha256.txt'

echo 'LAUNCH cuckoo md5...'
./bin/main cuckoo murmur3 bench/cuckoo-md5.txt $1
echo 'GENERATED bench/cuckoo-md5.txt'

echo 'LAUNCH cuckoo polynomial...'
./bin/main cuckoo murmur3 bench/cuckoo-polynomial.txt $1
echo 'GENERATED bench/cuckoo-polynomial.txt'

echo 'LAUNCH cuckoo tabulation...'
./bin/main cuckoo murmur3 bench/cuckoo-tabulation.txt $1
echo 'GENERATED bench/cuckoo-tabulation.txt'
