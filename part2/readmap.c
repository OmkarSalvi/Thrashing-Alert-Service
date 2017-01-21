#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

FILE *fm, *fmp;
char fmap[256], fpagemap[256], text[256];

int main( int argc, char *argv[])
{
   unsigned long long start, end, index, pfn_entry, vpn, fullAddr=0, searchAddr=0;
   int pages;

   sprintf(fmap,"/proc/%s/maps",argv[1]);
   sprintf(fpagemap,"/proc/%s/pagemap",argv[1]);

   if(argc < 2 || argc > 3)
   {
      printf("Usage: ./read_pagemap <pid> <hex_addr?>\n");
      exit(1);
   }

   if(argc == 3)
   {
      fullAddr = strtoull(argv[2], NULL, 16);
      searchAddr = (fullAddr / getpagesize()) * getpagesize();
   }

   if( !(fm = fopen(fmap, "r")) )
   {
      printf("Error opening map file. Invalid PID\n");
      exit(1);
   }

   if( !(fmp = fopen(fpagemap, "rb")) )
   {
      printf("Error opening pagemap file. Invalid PID\n");
      exit(1);
   }

   if(searchAddr == 0)
      printf("  VPN%-18sPFN\n", "");
   while( fgets(text, 256, fm) )
   {
      sscanf(text, "%llx-%llx", &start, &end);
      pages = (end - start) / 4096;
      index = (start / 4096) * sizeof( unsigned long long );

      if(searchAddr == 0)
         printf("start address: 0x %08llx, end address: 0x%08llx, number of pages:%d, index in pagemap: 0x%016llu\n", start, end, pages, index);

      fseek(fmp, index, SEEK_SET);

      vpn = start / 4096;
      while(pages--)
      {

         if( fread(&pfn_entry, 8, 1, fmp) )
         {
            // if we want an address and this isn't it then show nothing
            if(searchAddr != 0 && searchAddr != vpn * 4096)
            {
               vpn++;
               continue;
            }

            // if present...
            if( pfn_entry & 0x8000000000000000 )
            {
               printf("\tvirt_addr: 0x%08llx, vfn: %llu, pfn: %llu phys_addr:0x%llx \n", vpn * 4096, vpn, (pfn_entry & 0x7FFFFFFFFFFFFF), ((pfn_entry & 0x7FFFFFFFFFFFFF) << 12) + (fullAddr - searchAddr));
            } else if (pfn_entry & 0x4000000000000000 ) {
                printf("\tvirt_addr: 0x%08llx, vfn: %llu, swap_id: %llu, swap_offset: %llu \n", vpn * 4096, vpn, (pfn_entry & 0x7FFFFFFFFFFFFF), (pfn_entry & 0x7FFFFFFFFFFFFF) >> 5 );
            }
         }
         vpn++;
      }
      if(searchAddr == 0)
	printf("\n");
   }

   fclose(fm);
   fclose(fmp);

   return 0;
}