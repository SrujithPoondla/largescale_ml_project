#!/bin/bash

if [ ! -d ~/.ssh ]
then
  mkdir ~/.ssh
fi

if [ ! -f ~/.ssh/aws-key-$keyName ]; then
  ssh-keygen -t rsa -C "aws-key-$keyName" -f ~/.ssh/aws-key-$keyName -q -N ""
  chmod 600 ~/.ssh/aws-key-$keyName
  aws ec2 import-key-pair --key-name aws-key-$keyName --public-key-material file://$HOME/.ssh/aws-key-$keyName.pub
fi
