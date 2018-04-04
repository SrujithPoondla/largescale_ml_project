import asyncio
import aioredis

loop = asyncio.get_event_loop()

async def multi_set_key_redis(keys,values):
    redis = await aioredis.create_redis(
        'redis://localhost')

    async def transaction():
        tr = redis.multi_exec()
        for key, value in zip(keys, values):
            tr.set(key,value)
        await tr.execute()

        # assert result == await asyncio.gather(*keys)
        # return result

    await transaction()
    redis.close()
    await redis.wait_closed()

async def multi_get_key_redis(keys):
    redis = await aioredis.create_redis(
        'redis://localhost')
    result =[]
    async def transaction():
        tr = redis.multi_exec()
        for key in keys:
            tr.get(key)
        result.append(await tr.execute())

        # assert result == await asyncio.gather(*keys)
        # return result

    await transaction()
    redis.close()
    await redis.wait_closed()
    print(result)
    return result

loop.run_until_complete(multi_set_key_redis(["srujith", "teja"],["good","good"]))
loop.run_until_complete(multi_get_key_redis(["srujith","teja"]))

# will print 'value'

