# install python and gdown
sudo apt install python3-pip -y
pip3 install gdown

# download workload
echo "downloading workloads.tgz"
if [ ! -d "./workloads.tgz" ]; then
  python3 ./download_gdrive.py 1Ifd8AwQ5e6EMcm3l9yYn8tgI3qMwhRpb workloads.tgz
  # wget https://drive.google.com/uc?id=1Ifd8AwQ5e6EMcm3l9yYn8tgI3qMwhRpb&export=download -O workloads.tgz
fi

# https://drive.google.com/uc?id=1Ifd8AwQ5e6EMcm3l9yYn8tgI3qMwhRpb

echo "downloading micro-workloads.tgz"
if [ ! -d "./micro-workloads.tgz" ]; then
  python3 ./download_gdrive.py 1727S-g5j568BEgqMjc4zghT2_pz0EZhf micro-workloads.tgz
  # wget https://drive.google.com/uc?id=1727S-g5j568BEgqMjc4zghT2_pz0EZhf&export=download -O micro-workloads.tgz
fi

# decompress upd-workload
echo "downloading upd-workloads"
if [ ! -d "./upd-workloads" ]; then
  python3 ./download_gdrive.py 1CJjkswX08XqoF2RaxXBiKgWapjyMrXdi upd-workloads.tgz
  # wget https://drive.google.com/uc?id=1CJjkswX08XqoF2RaxXBiKgWapjyMrXdi&export=download -O upd-workloads.tgz
fi

# decompress workload
echo "decompressing workload files"
if [ ! -d "./workloads" ]; then
  tar zxvf workloads.tgz
fi

if [ ! -d "./micro-workloads" ]; then
  tar zxvf micro-workloads.tgz
fi

if [ ! -d "./upd-workloads" ]; then
  tar zxvf upd-workloads.tgz
fi