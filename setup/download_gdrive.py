import gdown
import sys

fid = sys.argv[1]
output = sys.argv[2]

url = "https://drive.google.com/uc?id={}&export=download".format(fid)

# url = "https://drive.google.com/open?id={}&authuser=0".format(fid)

# url = "https://drive.google.com/file/d/{}/view?usp=share_link".format(fid)

gdown.download(url, output, quiet=False)