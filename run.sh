#!/bin/sh

if [ $# -ne 1 ] 
then
    echo "USAGE: $0 TL_SECONDS"
    exit
fi

echo 'LAUNCH linear murmur3...'
./bin/main linear murmur3 bench/linear-murmur3.txt $1
echo 'GENERATED bench/linear-murmur3.txt'

echo 'LAUNCH quadro murmur3...'
./bin/main quadro murmur3 bench/quadro-murmur3.txt $1
echo 'GENERATED bench/quadro-murmur3.txt'

echo 'LAUNCH double murmur3...'
./bin/main double murmur3 bench/double-murmur3.txt $1
echo 'GENERATED bench/double-murmur3.txt'

echo 'LAUNCH chain75 murmur3...'
./bin/main chain75 murmur3 bench/chain75-murmur3.txt $1
echo 'GENERATED bench/chain75-murmur3.txt'

echo 'LAUNCH chain95 murmur3...'
./bin/main chain95 murmur3 bench/chain95-murmur3.txt $1
echo 'GENERATED bench/chain95-murmur3.txt'

echo 'LAUNCH cuckoo murmur3...'
./bin/main cuckoo murmur3 bench/cuckoo-murmur3.txt $1
echo 'GENERATED bench/cuckoo-murmur3.txt'
