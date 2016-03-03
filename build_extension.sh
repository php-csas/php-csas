#!/bin/sh

phpize
./configure
make
sudo make install
sudo service apache2 restart
