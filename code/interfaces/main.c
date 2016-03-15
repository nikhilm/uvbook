#include <stdio.h>
#include <uv.h>

#define _INT_BUF (512)

int main(void) {
    char buf[_INT_BUF];
    uv_interface_address_t *info;
    int count, i;

    uv_interface_addresses(&info, &count);

    printf("Number of interfaces: %d\n", count);
    for(i=count-1; i>=0; --i) {
        uv_interface_address_t interface = info[i];

        printf("Name: %s\n", interface.name);
        printf("Internal: %s\n", interface.is_internal ? "Yes" : "No");
        
        int family = interface.address.address4.sin_family;
        if (family == AF_INET) {
            uv_ip4_name(&interface.address.address4, buf, sizeof(buf));
            printf("IPv4 address: %s\n", buf);
        }
        else if (family == AF_INET6) {
            uv_ip6_name(&interface.address.address6, buf, sizeof(buf));
            printf("IPv6 address: %s\n", buf);
        }

        printf("\n");
    }

    uv_free_interface_addresses(info, count);
    return 0;
}
