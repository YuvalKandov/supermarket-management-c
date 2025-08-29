#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "main.h"
#include "General.h"
#include "Supermarket.h"
#include "SuperFile.h"


int main(int argc, char* argv[]) {
    SuperMarket market;
    srand((unsigned int)time(NULL));

    int compressedMode = atoi(argv[1]);
    char* fileName = argv[2];

    if (!initSuperMarket(&market, fileName, CUSTOMER_FILE_NAME, compressedMode)) {
        printf("Error initializing Super Market\n");
        return 0;
    }

    int option, stop = 0;
    do {
        option = menu();
        switch (option) {
        case eShowSuperMarket: 
            printSuperMarket(&market); 
            break;
        case eAddProduct: 
            if (!addProduct(&market)) printf("Error adding product\n"); 
            break;
        case eAddCustomer: 
            if (!addCustomer(&market)) printf("Error adding customer\n"); 
            break;
        case eCustomerDoShopping: 
            if (!doShopping(&market)) printf("Error in shopping\n"); 
            break;
        case ePrintCart: 
            doPrintCart(&market); 
            break;
        case eCustomerManageShoppingCart: 
            if (!manageShoppingCart(&market)) printf("Error in shopping cart management\n"); 
            break;
        case eSortProducts: 
            sortProducts(&market); 
            break;
        case eSearchProduct: 
            findProduct(&market); 
            break;
        case ePrintProductByType: 
            printProductByType(&market); 
            break;
        case EXIT: 
            printMessage("Thank", "You", "For", "Shopping", "With", "Us", NULL);
            stop = 1;
            break;
        default: 
            printf("Wrong option\n"); 
            break;
        }
    } while (!stop);

    handleCustomerStillShoppingAtExit(&market);

    if (!saveSuperMarketToFile(&market, fileName, CUSTOMER_FILE_NAME, compressedMode))
        printf("Error saving supermarket to file\n");

    freeMarket(&market);
    return 1;
}


int menu()
{
	int option;
	printf("\n");
	printf("Please choose one of the following options\n");
	for(int i = 0 ; i < eNofOptions ; i++)
		printf("%d - %s\n",i, menuStrings[i]);
	printf("%d - Quit\n", EXIT);
	scanf("%d", &option);
	//clean buffer
	char tav;
	scanf("%c", &tav);
	return option;
}

