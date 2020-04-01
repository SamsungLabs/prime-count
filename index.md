# Prime+Count: Novel Cross-world Covert Channels on ARM TrustZone

## Brief

- This repository contains codes for Prime+Count paper.
- You can try to build and test this covert channel between normal world and secure world.
- The source code in this repository is only for the normal world part. 
- The secure world part can be implemented by using the same approach depending on your environment.

## Prime+Count attack

- Prime+Count implements fast covert channels in single-core and cross-core scenarios in the TrustZone.
- Prime+Count achieves low-noise covert channel via monitoring how many L1/L2 cache refill events happens.
- To get full details, See the paper.

## How to build

- See comments in ./build.sh to get usage.
- Run ./build.sh with proper arguments.
- After ./build.sh, See output files under build/
- covert_lkm.ko :  kernel module for building covert channel
- main :  user application for testing

## How to test

- First of all, You have to update secure-os or trusted application running on secure-os.

  ```
  $ insmod covert_lkm.ko
  $ ./main 1 1 images/01/data /tmp/01.csv
    (See comments in src/user/main.c to get full usage and example.)
    (images/01/data :  input image to be transfered through Prime+Count covert channel)
    (/tmp/01.csv :  output encoded image, encoded via bucket method)
  ```

## Python script to convert bucket-encoded image to real image

- images/py_scrips/*.py are scripts to convert bucket-encoded image to real image.
- You can try to convert the bucket-encoded image with below instructions.
  ```
  $ cd images/py_scripts/test/
  $ python ../collect_image_set_counting_mode.py 01.csv (01.csv is prepared bucket-encoded image for testing)
  $ ls -l output.pbm
    (See output.pbm, and compare it to circle.pbm which is original image.)
  ```

## Image data

- images/ :  images for experiments.

## Full paper

- [Prime+Count: Novel Cross-world Covert Channels on ARM TrustZone](http://www.public.asu.edu/~hcho67/papers/prime+count-acsac18.pdf)

## License

- Under GPLv2.
- Copyrights are specificed under LICENSE.

## Contact

- Haehyun Cho <hcho67@asu.edu>
- Jinbum Park <jinb.park@samsung.com>, <jinb.park7@gmail.com>

