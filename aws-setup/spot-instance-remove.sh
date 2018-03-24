#!/bin/bash
instanceId=$(aws ec2 describe-spot-instance-requests --query 'SpotInstanceRequests[?SpotInstanceRequestId==`sir-vm884deh`].[InstanceId]' --output text)
aws ec2 disassociate-address --association-id eipassoc-82f40729
aws ec2 release-address --allocation-id eipalloc-c528becc
aws ec2 terminate-instances --instance-ids $instanceId
aws ec2 wait instance-terminated --instance-ids $instanceId
aws ec2 delete-network-interface --network-interface-id eni-a2516dbd
aws ec2 cancel-spot-instance-requests --spot-instance-request-ids sir-vm884deh
rm -f spot-instance-remove.sh
rm -f /Users/srujithpoondla/aws_scripts/spot-instance*
