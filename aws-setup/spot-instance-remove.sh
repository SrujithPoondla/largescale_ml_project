#!/bin/bash
instanceId=$(aws ec2 describe-spot-instance-requests --query 'SpotInstanceRequests[?SpotInstanceRequestId==`sir-pffr64sh`].[InstanceId]' --output text)
aws ec2 disassociate-address --association-id eipassoc-0e6597a5
aws ec2 release-address --allocation-id eipalloc-f99c0ff0
aws ec2 terminate-instances --instance-ids $instanceId
aws ec2 wait instance-terminated --instance-ids $instanceId
aws ec2 delete-network-interface --network-interface-id eni-84ffff9b
aws ec2 cancel-spot-instance-requests --spot-instance-request-ids sir-pffr64sh
rm -f spot-instance-remove.sh
rm -f /Users/srujithpoondla/aws_scripts/spot-instance*
