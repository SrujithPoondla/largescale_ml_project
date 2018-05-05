#!/bin/bash
if [ ! -d ~/aws_scripts ]
then
  mkdir ~/aws_scripts
fi
echo "#\!\/bin\/bash" > ~/aws_scripts/$instancePublicIp-connect.sh
echo ssh -i ~/.ssh/aws-key-$keyName ubuntu@$instancePublicIp > ~/aws_scripts/$instancePublicIp-connect.sh
