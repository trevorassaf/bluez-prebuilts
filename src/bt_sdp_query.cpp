
#include <cstdlib>
#include <iostream>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth.h>
#include <bluetooth/sdp.h>
#include <bluetooth/sdp_lib.h>

const bdaddr_t local_baddr{{0, 0, 0, 0, 0, 0}};

//		00001124-0000-1000-8000-00805f9b34fb
//		00001200-0000-1000-8000-00805f9b34fb

int main(int argc, char **argv)
{
  /*
    uint8_t svc_uuid_int[] =
    {
      0x00,
      0x00,
      0x11,
      0x24,
      0x00,
      0x00,
      0x10,
      0x80,
      0x00,
      0x00,
      0x80,
      0x5f, 
      0x9b,
      0x34,
      0xfb
    };
    */

  /*
    uint8_t svc_uuid_int[] =
    {
      0x00,
      0x00,
      0x12,
      0x00,
      0x00,
      0x00,
      0x10,
      0x80,
      0x00,
      0x00,
      0x80,
      0x5f, 
      0x9b,
      0x34,
      0xfb
    };
    */

  uint8_t svc_uuid_int[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0x11, 0x24 };

    uuid_t svc_uuid;
    int err;
    bdaddr_t target;
    sdp_list_t *response_list = NULL, *search_list, *attrid_list;
    sdp_session_t *session = 0;

    str2ba("C8:3F:26:11:D7:D3", &target );

    // connect to the SDP server running on the remote machine
    session = sdp_connect(&local_baddr, &target, SDP_RETRY_IF_BUSY );

    if (!session) {
      std::cerr << "Error: sdp_connect(): failure. " << strerror(errno) << std::endl;
      return EXIT_FAILURE;
    }

    // specify the UUID of the application we're searching for
    sdp_uuid128_create( &svc_uuid, &svc_uuid_int );
    search_list = sdp_list_append( NULL, &svc_uuid );

    if (!search_list)
    {
      std::cerr << "Error: search_list is null" << std::endl;
    }

    // specify that we want a list of all the matching applications' attributes
    //uint32_t range = 0x0000ffff;
    uint32_t range = 0x0000ffff;
    attrid_list = sdp_list_append( NULL, &range );

    // get a list of service records that have UUID 0xabcd
    err = sdp_service_search_attr_req( session, search_list, \
            SDP_ATTR_REQ_RANGE, attrid_list, &response_list);

    if (err == 0)
    {
      std::cerr << "No records returned. But operation finished successfully" << std::endl;
      //return EXIT_FAILURE;
    }
    else if (err == -1)
    {
      std::cerr << "Operation timed out. " << std::endl;
      return EXIT_FAILURE;
    }

    std::cout << "Operation succeeded and here are the results..." << std::endl;

    sdp_list_t *r = response_list;

    if (!r)
    {
      std::cerr << "SDP results is null. Quitting..." << std::endl;
      return EXIT_FAILURE;
    }

    // go through each of the service records
    std::cout << "Dumping SDP records..." << std::endl;

    size_t num_records = 0;
    for (; r; r = r->next ) {
      std::cout << "SDP record " << num_records++ << std::endl;

        sdp_record_t *rec = (sdp_record_t*) r->data;
        sdp_list_t *proto_list;

        // get a list of the protocol sequences
        if( sdp_get_access_protos( rec, &proto_list ) == 0 ) {
        sdp_list_t *p = proto_list;

        // go through each protocol sequence
        for( ; p ; p = p->next ) {
            sdp_list_t *pds = (sdp_list_t*)p->data;

            // go through each protocol list of the protocol sequence
            for( ; pds ; pds = pds->next ) {

                // check the protocol attributes
                sdp_data_t *d = (sdp_data_t*)pds->data;
                int proto = 0;
                for( ; d; d = d->next ) {
                    switch( d->dtd ) {
                        case SDP_UUID16:
                        case SDP_UUID32:
                        case SDP_UUID128:
                            proto = sdp_uuid_to_proto( &d->val.uuid );
                            break;
                        case SDP_UINT8:
                            if( proto == RFCOMM_UUID ) {
                                printf("rfcomm channel: %d\n",d->val.int8);
                            }
                            break;
                    }
                }
            }
            sdp_list_free( (sdp_list_t*)p->data, 0 );
        }
        sdp_list_free( proto_list, 0 );

        }

        printf("found service record 0x%x\n", rec->handle);
        sdp_record_free( rec );
    }

    std::cout << "Finished dumping SDP records..." << std::endl;

    sdp_close(session);
    return EXIT_SUCCESS;
}
