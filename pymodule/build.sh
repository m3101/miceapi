sudo rm -rf build
sudo rm -rf dist
echo "Generating python wheels..."
python3 setup.py bdist_wheel
echo "Converting wheels to manylinux..."
sudo docker run -v ~/Documents/Projects/MultiMouseApi/pymodule/dist:/mnt/dist quay.io/pypa/manylinux2010_x86_64 /bin/bash -c "cd /mnt/dist;for wheel in \$(ls);do auditwheel repair \$wheel;done"
echo "Done!"