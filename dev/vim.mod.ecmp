[info]
name = vim
version = 9.0.0228
type = src
license = MIT
url = https://anduin.linuxfromscratch.org/BLFS/vim/vim-9.0.0228.tar.gz

[download]
curl $URL | tar -xz 

[install]
./configure --prefix=/usr
make -j$(nproc)
make DESTDIR=$BUILD_ROOT install

[special]
echo "the package is installed"
echo "done..."

[description]
This package is really important.
It is essential to the system.
It is a dependency of many other packages.

