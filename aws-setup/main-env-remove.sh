#!/bin/bash
aws ec2 delete-security-group --group-id sg-693e661f
aws ec2 disassociate-route-table --association-id rtbassoc-f8a55287
aws ec2 delete-route-table --route-table-id rtb-9b41c0e7
aws ec2 detach-internet-gateway --internet-gateway-id igw-76e44b0e --vpc-id vpc-d4bd28af
aws ec2 delete-internet-gateway --internet-gateway-id igw-76e44b0e
aws ec2 delete-subnet --subnet-id subnet-2ba29e76
aws ec2 delete-vpc --vpc-id vpc-d4bd28af
rm -f /Users/srujithpoondla/aws_scripts/authorize-current-ip /Users/srujithpoondla/aws_scripts/list-instances /Users/srujithpoondla/aws_scripts/deauthorize-ip /Users/srujithpoondla/aws_scripts/list-authorized-ips /Users/srujithpoondla/aws_scripts/cancel-open-spot-instance-requests /Users/srujithpoondla/aws_scripts/list-open-spot-instance-requests /Users/srujithpoondla/aws_scripts/list-active-spot-instance-requests
rm -f main-env-remove.sh main-env-vars.sh
