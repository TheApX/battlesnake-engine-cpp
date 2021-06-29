CONFIG=Release
PARALLEL=4

cmake --config ${CONFIG} -H. -B./build && \
cmake --build ./build --config ${CONFIG} --target all -j ${PARALLEL} && \
cd ./build && \
ctest --config ${CONFIG} -j ${PARALLEL}
