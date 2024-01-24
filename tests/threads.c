#include "test.h"

/*--------------------------------------------------------------------------------------------------------------------------
 *
 *                                                 Tests for Threads
 *
 *-------------------------------------------------------------------------------------------------------------------------*/
typedef struct
{
    CoyChannel *outbound;
    u64 num_to_send;
} ProducerThreadData;

typedef struct
{
    CoyChannel *inbound;
    u64 num_received;
} ConsumerThreadData;

static CoyThreadFunReturnType
producer(void *data)
{
    ProducerThreadData *pdata = data;
    CoyChannel *outbound = pdata->outbound;
    u64 num_to_send = pdata->num_to_send;

    coy_channel_register_sender(outbound);
    coy_channel_wait_until_ready_to_send(outbound);

    bool success = true;
    for(u64 i = 0; i < num_to_send && success; ++i)
    {
        success = coy_channel_send(outbound, (void *) 1);
    }

    coy_channel_done_sending(outbound);
    
    return (CoyThreadFunReturnType)1;
}

static CoyThreadFunReturnType
consumer(void *data)
{
    ConsumerThreadData *cdata = data;
    CoyChannel *inbound = cdata->inbound;
    u64 sum = 0;

    coy_channel_register_receiver(inbound);
    coy_channel_wait_until_ready_to_receive(inbound);

    void *val;
    while(coy_channel_receive(inbound, &val))
    {
        sum += (u64)val;
    }

    coy_channel_done_receiving(inbound);
    cdata->num_received = sum;

    return (CoyThreadFunReturnType)1;
}

#define NUM_TO_SEND 1000000

static void
test_single_producer_single_consumer(void)
{
    CoyChannel chan = coy_channel_create();

    ProducerThreadData pdata = { .outbound = &chan, .num_to_send = NUM_TO_SEND };
    CoyThread producer_thread = coy_thread_create(producer, &pdata);

    ConsumerThreadData cdata = { .inbound = &chan, .num_received = 0 };
    CoyThread consumer_thread = coy_thread_create(consumer, &cdata);

    bool success = coy_thread_join(&producer_thread);
    Assert(success);

    success = coy_thread_join(&consumer_thread);
    Assert(success);

    coy_channel_destroy(&chan, NULL, NULL);

    Assert(cdata.num_received == NUM_TO_SEND);
}

static void
test_single_producer_multiple_consumer(void)
{
    CoyChannel chan = coy_channel_create();

    ProducerThreadData pdata = { .outbound = &chan, .num_to_send = NUM_TO_SEND };
    CoyThread producer_thread = coy_thread_create(producer, &pdata);

    ConsumerThreadData cdata[4] = {0};
    CoyThread consumer_threads[4] = {0};
    for(i32 i = 0; i < 4; ++i)
    {
        cdata[i] = (ConsumerThreadData){ .inbound = &chan, .num_received = 0 };
        consumer_threads[i] = coy_thread_create(consumer, &cdata[i]);
    }

    bool success = coy_thread_join(&producer_thread);
    Assert(success);

    for(i32 i = 0; i < 4; ++i)
    {
        success = coy_thread_join(&consumer_threads[i]);
        Assert(success);
    }

    coy_channel_destroy(&chan, NULL, NULL);

    u64 total = 0;
    for(i32 i = 0; i < 4; ++i)
    {
        total += cdata[i].num_received;
    }

    Assert(total == NUM_TO_SEND);
}

static void
test_multiple_producer_single_consumer(void)
{
    CoyChannel chan = coy_channel_create();

    ProducerThreadData pdata[4] = {0};
    CoyThread producer_threads[4] = {0};
    for(i32 i = 0; i < 4; ++i)
    {
        pdata[i] = (ProducerThreadData){ .outbound = &chan, .num_to_send = NUM_TO_SEND };
        producer_threads[i] = coy_thread_create(producer, &pdata[i]);
    }

    ConsumerThreadData cdata = { .inbound = &chan, .num_received = 0 };
    CoyThread consumer_thread = coy_thread_create(consumer, &cdata);
    

    bool success = true;
    for(i32 i = 0; i < 4; ++i)
    {
        success = coy_thread_join(&producer_threads[i]);
        Assert(success);
    }

    success = coy_thread_join(&consumer_thread);
    Assert(success);

    coy_channel_destroy(&chan, NULL, NULL);

    Assert(cdata.num_received == 4 * NUM_TO_SEND);
}

static void
test_multiple_producer_multiple_consumer(void)
{
    CoyChannel chan = coy_channel_create();

    ProducerThreadData pdata[4] = {0};
    CoyThread producer_threads[4] = {0};
    for(i32 i = 0; i < 4; ++i)
    {
        pdata[i] = (ProducerThreadData){ .outbound = &chan, .num_to_send = NUM_TO_SEND };
        producer_threads[i] = coy_thread_create(producer, &pdata[i]);
    }

    ConsumerThreadData cdata[4] = {0};
    CoyThread consumer_threads[4] = {0};
    for(i32 i = 0; i < 4; ++i)
    {
        cdata[i] = (ConsumerThreadData){ .inbound = &chan, .num_received = 0 };
        consumer_threads[i] = coy_thread_create(consumer, &cdata[i]);
    }

    bool success = true;
    for(i32 i = 0; i < 4; ++i)
    {
        success = coy_thread_join(&producer_threads[i]);
        Assert(success);
    }

    for(i32 i = 0; i < 4; ++i)
    {
        success = coy_thread_join(&consumer_threads[i]);
        Assert(success);
    }

    coy_channel_destroy(&chan, NULL, NULL);

    u64 total = 0;
    for(i32 i = 0; i < 4; ++i)
    {
        total += cdata[i].num_received;
    }

    Assert(total == 4 * NUM_TO_SEND);
}

/*---------------------------------------------------------------------------------------------------------------------------
 *                                                   All threads tests
 *-------------------------------------------------------------------------------------------------------------------------*/
void
coyote_threads_tests(void)
{
    fprintf(stderr,".spsc..");
    test_single_producer_single_consumer();
    fprintf(stderr,".spmc..");
    test_single_producer_multiple_consumer();
    fprintf(stderr,".mpsc..");
    test_multiple_producer_single_consumer();
    fprintf(stderr,".mpmc..");
    test_multiple_producer_multiple_consumer();
}

