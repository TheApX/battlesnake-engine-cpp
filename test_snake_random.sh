CONFIG=Release
PARALLEL=4

cmake --config ${CONFIG} -H. -B./build && \
cmake --build ./build --config ${CONFIG} --target battlesnake_random_test -j ${PARALLEL} && \
cd ./build/snakes/random/ && \
./battlesnake_random_test
