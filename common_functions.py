import aioredis


async def multi_set_key_redis(keys,values):
    redis = await aioredis.create_redis(
        'redis://localhost')

    async def transaction():
        tr = redis.multi_exec()
        for key, value in zip(keys, values):
            tr.set(key,value)
        await tr.execute()
    await transaction()
    redis.close()
    await redis.wait_closed()


async def multi_get_key_redis(keys):
    redis = await aioredis.create_redis_pool(
        'redis://localhost')
    result =[]
    for key in keys:
        result.append(await redis.get(key))
    redis.close()
    await redis.wait_closed()
    print(result)
    return result


def push_params_redis(model,loop):
    # loop = asyncio.get_event_loop()
    i = -1
    keys=[]
    values=[]
    for param in list(model.parameters()):
        i = i+1
        param_data = param.data.numpy()
        # p = pc._dumps(param_data, protocol=pc.HIGHEST_PROTOCOL)
        keys.append(i)
        values.append(str(param_data))
    loop.run_until_complete(multi_set_key_redis(keys, values))


def get_params_redis(shapes_len,loop):
    # loop = asyncio.get_event_loop()
    keys = list(range(shapes_len))
    params = loop.run_until_complete(multi_get_key_redis(keys))
    return params


def get_shapes_len(model):
    shapes = []
    for param in list(model.parameters()):
        shapes.append(list(param.size()))
    return len(shapes)


def set_params(model, params):
    for param, new_param in zip(list(model.parameters()), params):
        param.data = new_param.data

# def push_params_memcache(model, client):
#     i = -1
#     for param in list(model.parameters()):
#         i = i + 1
#         param_data = param.data.numpy()
#         p = pc._dumps(param_data, protocol=pc.HIGHEST_PROTOCOL)
#         client.set(str(i), p)


# def get_params_memcache(client, shapes):
#     i = -1
#     params=[]
#     for shape in shapes:
#         i = i + 1
#         param = client.get(str(i))
#         param_np = pc._loads(param).reshape(shape)
#         param_tensor = torch.nn.Parameter(torch.from_numpy(param_np))
#         params.append(param_tensor)
#     return params
