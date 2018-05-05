#!/bin/bash
export vpcId="main-env"
export availabilityZone="us-east-1a"

export vpcId=`aws ec2 create-vpc --cidr-block 10.0.0.0/28 --query 'Vpc.VpcId' --output text`
aws ec2 create-tags --resources $vpcId --tags --tags Key=Name,Value=$vpcId
aws ec2 modify-vpc-attribute --vpc-id $vpcId --enable-dns-support "{\"Value\":true}"
aws ec2 modify-vpc-attribute --vpc-id $vpcId --enable-dns-hostnames "{\"Value\":true}"

export internetGatewayId=`aws ec2 create-internet-gateway --query 'InternetGateway.InternetGatewayId' --output text`
aws ec2 create-tags --resources $internetGatewayId --tags --tags Key=Name,Value=$vpcId-gateway
aws ec2 attach-internet-gateway --internet-gateway-id $internetGatewayId --vpc-id $vpcId

export subnetId=`aws ec2 create-subnet --vpc-id $vpcId --cidr-block 10.0.0.0/28  --availability-zone $availabilityZone --query 'Subnet.SubnetId' --output text`
aws ec2 create-tags --resources $internetGatewayId --tags --tags Key=Name,Value=$vpcId-subnet

export routeTableId=`aws ec2 create-route-table --vpc-id $vpcId --query 'RouteTable.RouteTableId' --output text`
aws ec2 create-tags --resources $routeTableId --tags --tags Key=Name,Value=$vpcId-route-table
export routeTableAssoc=`aws ec2 associate-route-table --route-table-id $routeTableId --subnet-id $subnetId --output text`
aws ec2 create-route --route-table-id $routeTableId --destination-cidr-block 0.0.0.0/0 --gateway-id $internetGatewayId

export securityGroupId=`aws ec2 create-security-group --group-name $vpcId-security-group --description "SG for main-env" --vpc-id $vpcId --query 'GroupId' --output text`
# ssh

# save delete commands for cleanup
echo "#!/bin/bash" > $vpcId-remove.sh # overwrite existing file

echo aws ec2 delete-security-group --group-id $securityGroupId >> $vpcId-remove.sh
echo aws ec2 disassociate-route-table --association-id $routeTableAssoc >> $vpcId-remove.sh
echo aws ec2 delete-route-table --route-table-id $routeTableId >> $vpcId-remove.sh
echo aws ec2 detach-internet-gateway --internet-gateway-id $internetGatewayId --vpc-id $vpcId >> $vpcId-remove.sh
echo aws ec2 delete-internet-gateway --internet-gateway-id $internetGatewayId >> $vpcId-remove.sh
echo aws ec2 delete-subnet --subnet-id $subnetId >> $vpcId-remove.sh
echo aws ec2 delete-vpc --vpc-id $vpcId >> $vpcId-remove.sh

echo rm -f ~/aws_scripts/authorize-current-ip ~/aws_scripts/list-instances ~/aws_scripts/deauthorize-ip ~/aws_scripts/list-authorized-ips ~/aws_scripts/cancel-open-spot-instance-requests ~/aws_scripts/list-open-spot-instance-requests ~/aws_scripts/list-active-spot-instance-requests >> $vpcId-remove.sh
echo rm -f $vpcId-remove.sh $vpcId-vars.sh >> $vpcId-remove.sh
chmod +x $vpcId-remove.sh

# Save variables
echo "#!/bin/bash" > $vpcId-vars.sh # overwrite existing file
echo export subnetId=$subnetId >> $vpcId-vars.sh
echo export securityGroupId=$securityGroupId >> $vpcId-vars.sh
echo export routeTableId=$routeTableId >> $vpcId-vars.sh
echo export envName=$vpcId >> $vpcId-vars.sh
echo export vpcId=$vpcId >> $vpcId-vars.sh
echo export internetGatewayId=$internetGatewayId >> $vpcId-vars.sh
echo export subnetId=$subnetId >> $vpcId-vars.sh
echo export routeTableAssoc=$routeTableAssoc >> $vpcId-vars.sh
echo export availabilityZone=$availabilityZone >> $vpcId-vars.sh

# Create maintenance scripts
if [ ! -d ~/aws_scripts ]
then
  mkdir ~/aws_scripts
fi

echo echo \$\(aws ec2 describe-security-groups --query \'SecurityGroups[?GroupId==\`$securityGroupId\`].IpPermissions[*].[IpRanges]\' --output text\) > ~/aws_scripts/list-authorized-ips
echo authorizedIp=\$\(aws ec2 describe-security-groups --query \'SecurityGroups[?GroupId==\`$securityGroupId\`].IpPermissions[*].[IpRanges][0]\' --output text\) > ~/aws_scripts/deauthorize-ip
echo aws ec2 revoke-security-group-ingress --group-id $securityGroupId --protocol tcp --port 22 --cidr '$authorizedIp' >> ~/aws_scripts/deauthorize-ip
echo aws ec2 revoke-security-group-ingress --group-id $securityGroupId --protocol tcp --port 8888-8898 --cidr '$authorizedIp' >> ~/aws_scripts/deauthorize-ip
echo externalIP='$(dig +short myip.opendns.com @resolver1.opendns.com)' > ~/aws_scripts/authorize-current-ip
echo aws ec2 authorize-security-group-ingress --group-id $securityGroupId --protocol tcp --port 22 --cidr '$externalIP'/32 >> ~/aws_scripts/authorize-current-ip
echo aws ec2 authorize-security-group-ingress --group-id $securityGroupId --protocol tcp --port 8888-8898 --cidr '$externalIP'/32 >> ~/aws_scripts/authorize-current-ip
echo aws ec2 describe-instances --query "'Reservations[*].Instances[*].{ID:InstanceId, type:InstanceType, state:State.Name, IP: PublicIpAddress, DNSName: PublicDnsName}'" --output text > ~/aws_scripts/list-instances
echo export spotInstanceRequestIds=\$\(aws ec2 describe-spot-instance-requests --query "'SpotInstanceRequests[?State==\`open\`].[SpotInstanceRequestId]'" --output text\) > ~/aws_scripts/cancel-open-spot-instance-requests
echo aws ec2 cancel-spot-instance-requests --spot-instance-request-ids \$spotInstanceRequestIds >> ~/aws_scripts/cancel-open-spot-instance-requests
echo aws ec2 describe-spot-instance-requests --query "'SpotInstanceRequests[?State==\`open\`].{type:[LaunchSpecification][0].[InstanceType][0],status:Status,requestId:SpotInstanceRequestId,price:SpotPrice}'" > ~/aws_scripts/list-open-spot-instance-requests
echo aws ec2 describe-spot-instance-requests --query "'SpotInstanceRequests[?State==\`active\`].{type:[LaunchSpecification][0].[InstanceType][0],status:Status,requestId:SpotInstanceRequestId,price:SpotPrice}'" > ~/aws_scripts/list-active-spot-instance-requests

chmod +x ~/aws_scripts/authorize-current-ip ~/aws_scripts/list-instances ~/aws_scripts/deauthorize-ip ~/aws_scripts/list-authorized-ips ~/aws_scripts/cancel-open-spot-instance-requests ~/aws_scripts/list-open-spot-instance-requests ~/aws_scripts/list-active-spot-instance-requests
