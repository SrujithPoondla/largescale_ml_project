# import sys
# import boto.vpc
# import boto.ec2
# from boto.regioninfo import get_regions
# from boto.vpc import RegionData, regions, RegionInfo
#
# VERBOSE =1
#
# # def get_region(region):
# #     for reg in boto.vpc.regions():
# #         if reg.name == region:
# #             return reg
#
# def get_regions():
#   """ Build a region list """
#
#   reg_list = []
#   for reg in boto.vpc.regions():
#     if reg.name == 'us-gov-west-1' or reg.name == 'cn-north-1':
#       continue
#     reg_list.append(reg)
#
#   return reg_list
#
# def del_igw(conn, vpcid):
#     """ Detach and delete the internet-gateway """
#
#     igw = conn.get_all_internet_gateways(filters={'attachment.vpc-id': vpcid})
#     if igw:
#         try:
#             print("Removing igw-id: ", igw[0].id) if (VERBOSE == 1) else ""
#             conn.detach_internet_gateway(igw[0].id, vpcid)
#             status = conn.delete_internet_gateway(igw[0].id)
#         except boto.exception.EC2ResponseError as e:
#             print(e.message)
#
#
# def del_sub(conn, vpcid):
#     """ Delete the subnets """
#
#     subs = conn.get_all_subnets(filters={'vpcId': [vpcid]})
#     if subs:
#         try:
#             for sub in subs:
#                 print("Removing sub-id: ", sub.id) if (VERBOSE == 1) else ""
#                 status = conn.delete_subnet(sub.id)
#         except boto.exception.EC2ResponseError as e:
#             print(e.message)
#
#
# def del_rtb(conn, vpcid):
#     """ Delete the route-tables """
#
#     rtbs = conn.get_all_route_tables(filters={'vpc-id': vpcid})
#     if rtbs:
#         try:
#             for tbl in rtbs:
#                 for assoc in tbl.associations:
#                     main = 'true' if (assoc.main == True) else 'false'
#                 if main == 'true':
#                     continue
#                 print("Removing rtb-id: ", tbl.id) if (VERBOSE == 1) else ""
#                 status = conn.delete_route_table(tbl.id)
#         except boto.exception.EC2ResponseError as e:
#             print(e.message)
#
#
# def del_acl(conn, vpcid):
#     """ Delete the network-access-lists """
#
#     acls = conn.get_all_network_acls(filters={'vpc-id': vpcid})
#     if acls:
#         try:
#             for acl in acls:
#                 if acl.default == 'true':
#                     continue
#                 print("Removing acl-id: ", acl.id) if (VERBOSE == 1) else ""
#                 status = conn.delete_network_acl(acl.id)
#         except boto.exception.EC2ResponseError as e:
#             print(e.message)
#
#
# def del_sgp(conn, vpcid):
#     """ Delete any security-groups """
#
#     sgps = conn.get_all_security_groups(filters={'vpc-id': vpcid})
#     if sgps:
#         try:
#             for sg in sgps:
#                 if sg.name == 'default':
#                     continue
#                 print("Removing sgp-id: ", sg.id) if (VERBOSE == 1) else ""
#                 status = conn.delete_security_group(group_id=sg.id)
#         except boto.exception.EC2ResponseError as e:
#             print(e.message)
#
#
# def del_vpc(conn, vpcid):
#     try:
#         print("Removing vpc-id: ", vpcid)
#         status = conn.delete_vpc(vpcid)
#     except boto.exception.EC2ResponseError as e:
#         print(e.message)
#         print("Please remove dependencies and delete VPC manually.")
#         # finally:
#         #  return statu
#
#
# # def main(keyid, secret, region):
#     # try:
#     #     conn = boto.vpc.VPCConnection(aws_access_key_id=keyid, aws_secret_access_key=secret, region=region)
#     #     attributes = conn.get_all_vpcs()
#     # except boto.exception.EC2ResponseError as e:
#     #     print(e.message)
#     #     exit(1)
#     #
#     #
# def main(keyid, secret):
#     regions = get_regions()
#
#     for region in regions:
#         try:
#             conn = boto.vpc.VPCConnection(aws_access_key_id=keyid, aws_secret_access_key=secret, region=region)
#             attributes = conn.describe_account_attributes(attribute_names='get-all-vpc')
#         except boto.exception.EC2ResponseError as e:
#             print(e.message)
#             exit(1)
#         else:
#             print("\n" + region.name.upper())
#
#             for attribute in attributes:
#                 default = attribute.attribute_values[0]
#                 print("Default  vpc-id: ", default)
#
#     for attribute in attributes:
#         if attribute != 'none':
#             del_igw(conn, attribute)
#             del_sub(conn, attribute)
#             del_rtb(conn, attribute)
#             del_acl(conn, attribute)
#             del_sgp(conn, attribute)
#             del_vpc(conn, attribute)
#
#
# if __name__ == "__main__":
#     # main(keyid='AKIAI6KJQR7DB64CIJFA', secret='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',
#     #      region=get_region('us-east-1'))
#     main(keyid='AKIAI6KJQR7DB64CIJFA', secret='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')


from spotcheck import spotcheck

print(spotcheck.main('t1.micro'))