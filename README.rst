BlockSci
~~~~~~~~~~~~~~~~~~

The Bitcoin blockchain — currently 140GB and growing — contains a massive amount of data that can give us insights into the Bitcoin ecosystem, including how users, businesses, and miners operate. BlockSci enables fast and expressive analysis of Bitcoin’s and many other blockchains. The accompanying working paper explains its design and applications: https://arxiv.org/pdf/1709.02489.pdf

Current tools for blockchain analysis depend on general-purpose databases that provide "ACID" guarantees. But that’s unnecessary for blockchain analysis where the data structures are append-only. We take advantage of this observation in the design of our custom in-memory blockchain database as well as an analysis library. BlockSci’s core infrastructure is written in C++ and optimized for speed. (For example, traversing every transaction input and output on the Bitcoin blockchain takes only 10.3 seconds on our r4.2xlarge EC2 machine.) To make analysis more convenient, we provide Python bindings and a Jupyter notebook interface. 

Documentation
=====================
Documentation_ is available for the python interface library.

.. _Documentation: https://citp.github.io/BlockSci/

Additionally, a demonstration Notebook_ is available in the Notebooks folder.

.. _Notebook: https://citp.github.io/BlockSci/demo.html

For installation instructions, see below. More detailed documentation is coming soon. Meanwhile, feel free to contact us at blocksci@lists.cs.princeton.edu.

Quick Setup using Amazon EC2
==============

If you want to start using BlockSci immediately, we have made available an EC2 image: ami-1f9b8364_. We recomend using instance with 60 GB of memory or more for optimal performance (r4.2xlarge). On boot, a Jupyter Notebook running BlockSci will launch immediately. To access the notebook, you must set up port forwarding to your computer. Inserting the name of your private key file and the domain of your ec2 instance into the following command will make the Notebook available on your machine.

.. code-block:: bash

	ssh -i .ssh/your_private_key.pem -N -L 8888:localhost:8888 ubuntu@your_url.amazonaws.com

This sets up an SSH tunnel between port 8888 on your remote EC2 instance and port 8888 on your localhost. You can use whichever port you like on your local machine. Next, you can navigate to https://localhost:8888/ (https is required) in your browser and log in with the password 'blocksci'. When connecting to the server, you will receive a certificate error regarding the self-signed certificate. This error is expected and can be safely bypassed. A demo notebook will be available for you to run and you can begin exploring the blockchain. Don't forget to shut down the EC2 instance when you are finished since EC2 charges hourly.

AWS instances suffer from a `known performance issue`_ when starting up from an existing AMI. When the machine starts up it doesn't actually load all of the data on the disk so that startup can be instant. Instead it only loads the data when it is accessed for the first time. Thus without intervention BlockSci will be substantially slower than the benchmarks report when commands are run the first time although subsequent queries will be drastically faster.

If you want to eliminate this performance issue entirely, you can do so using the ``fio`` utility which takes about 12 hours to run.

You'll first need to determine which block device the disk is loaded as using the ``lsblk`` tool. There should only be one devlice listed which is ``xvda`` on our instance. In the final line replace ``xvda`` with the block device listed by ``lsblk``. Further instructions are available from Amazon_.

.. code-block:: bash

	ssh -i .ssh/your_private_key.pem ubuntu@your_url.amazonaws.com
	lsblk
	sudo apt-get install -y fio
	sudo fio --filename=/dev/xvda --rw=read --bs=128k --iodepth=32 --ioengine=libaio --direct=1 --name=volume-initialize

.. _ami-1f9b8364: https://console.aws.amazon.com/ec2/home?region=us-east-1#launchAmi=ami-1f9b8364
.. _known performance issue: https://docs.aws.amazon.com/AWSEC2/latest/UserGuide/ebs-initialize.html
.. _Amazon: https://docs.aws.amazon.com/AWSEC2/latest/UserGuide/ebs-initialize.html

Local Setup
=====================
To run BlockSci locally, you must be running a full node (such as bitcoind or an altcoin node) since BlockSci requires the full serialized blockchain data structure which full nodes produce. 

Parser
----------

The next step is parsing the blockchain downloaded by the full node. The blockchain_parser binary which is built from this repo performs the transformation of this blockchain data into our customized BlockSci database format. It has two modes.

Disk mode is optimized for parsing Bitcoin's data files. It reads blockchain data directly from disk in a rapid manner. However this means that it does not work on many other blockchains which have different serialization formats than Bitcoin.

..  code-block:: bash

	blocksci_parser --coin-directory .bitcoin --output-directory bitcoin-data

RPC mode uses the RPC interface of a cryptocurrency to extract data regarding the blockchain. It works with a variety of cryptocurrencies which have the same general model as Bitcoin, but with minor changes to the serialization format which break the parser in disk mode. One example of this is Namecoin.

..  code-block:: bash

	blocksci_parser --username [user] --password [pass] --address [ip] --port [port] --output-directory bitcoin-data

Using the Analysis Library
============================

After the parser has been run, the analysis library is ready for use. This can again be used through two different interfaces

C++
------

In order to use the C++ library, you must compile your code against the BlockSci dynamic library and add its headers to your include path. The Blockchain can then be constructed given the path to the output of the parser.

.. code-block:: c++

	#include <blocksci/chain/blockchain.hpp>
	
	int main(int argc, const char * argv[]) {
    		blocksci::Blockchain chain{"file_path_to_output-directory"};
	}

Python
-------

To use the BlockSci in python, you only need to import the BlockSci library. By default the library is installed into BlockSci/Notebooks. To use the library first open the Python interpreter in that folder:

.. code-block:: bash

	cd BlockSci/Notebooks
	python3
	
With the python interpretter open, the following code will load a Blockchain object created from the data output by the parser:

.. code-block:: python

	import blocksci
	chain = blocksci.Blockchain("file_path_to_parser_output-directory")

If you would like to use BlockSci through a web interface, we recommend the use of `Jupyter Notebook`_. Once Jupyter is installed, simply navigate into BlockSci/Notebooks and run:

.. code-block:: bash

	jupyter notebook
	
which will open a window in your browser to the Jupyter server.

.. _Jupyter Notebook: https://jupyter.readthedocs.io/en/latest/install.html


BlockSci Compilation Instructions
======================================

Here are the steps for compiling BlockSci on Ubuntu 16.04.

Note that BlockSci only actively supports python 3.

..  code-block:: bash

	sudo apt-get update
	sudo apt install build-essential cmake libssl-dev libboost-all-dev libsqlite3-dev autogen \
	autoconf libleveldb-dev libcurl4-openssl-dev libjsoncpp-dev libjsonrpccpp-dev libjsonrpccpp-tools \
	python3-dev python3-pip
	sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y
	sudo apt-get update
	sudo apt install gcc-6 g++-6
	sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-6 60 --slave /usr/bin/g++ g++ /usr/bin/g++-6

	git clone https://github.com/bitcoin-core/secp256k1
	cd secp256k1
	./autogen.sh
	./configure --enable-module-recovery
	make
	sudo make install
	
	git clone https://github.com/citp/BlockSci.git
	cd BlockSci
	git submodule update --init --recursive --checkout

	cd libs/bitcoin-api-cpp
	mkdir release
	cd release
	cmake --DCMAKE_BUILD_TYPE=Release ..
	make
	sudo make install

	cd ../../..
	mkdir release
	cd release
	cmake --DCMAKE_BUILD_TYPE=Release ..
	make
	sudo make install

	sudo -H pip3 install --upgrade pip
	sudo -H pip3 install --upgrade multiprocess psutil jupyter pycrypto matplotlib pandas dateparser
	
Team
=======

BlockSci was created by Harry Kalodner, Steven Goldfeder, Alishah Chator, Malte Möser, and Arvind Narayanan at Princeton University. It is supported by NSF grants CNS-1421689 and CNS-1651938 and an NSF Graduate Research Fellowship under grant number DGE-1148900. We've released a paper_ describing BlockSci's design and a few applications that illustrate its capabilities. You can contact the team at blocksci@lists.cs.princeton.edu.

.. _paper: https://arxiv.org/abs/1709.02489
