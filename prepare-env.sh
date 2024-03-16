sudo rm -rf software-prep
mkdir  -p software-prep
cd software-prep

# for fmt
apt install -y wget
wget https://github.com/fmtlib/fmt/releases/download/10.0.0/fmt-10.0.0.zip
apt install -y unzip
unzip fmt-10.0.0.zip
cd fmt-10.0.0
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j`nproc`
make install

sudo apt-get -y install liburing-dev

cd ../../




