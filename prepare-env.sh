sudo rm -rf software-prep
mkdir  -p software-prep
cd software-prep

#wget https://github.com/fmtlib/fmt/releases/download/10.0.0/fmt-10.0.0.zip
#unzip fmt-10.0.0.zip
#cd fmt-10.0.0
#mkdir build
#cd build
#cmake -DCMAKE_BUILD_TYPE=Release ..
#make -j`nproc`
#make install
#



if lsmod | grep -q "io_uring"; then
  echo "io_uring module is loaded."
else
  https://github.com/axboe/liburing/archive/refs/tags/liburing-2.4.tar.gz
fi


cd ../../