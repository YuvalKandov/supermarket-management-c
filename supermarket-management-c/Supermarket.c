#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "Supermarket.h"
#include "Product.h"
#include "Customer.h"
#include "ClubMember.h"
#include "General.h"
#include "ShoppingCart.h"
#include "FileHelper.h"
#include "SuperFile.h"
#include "myMacros.h"

static const char* sortOptStr[eNofSortOpt] = { "None", "Name", "Count", "Price" };

int initSuperMarket(SuperMarket* pMarket, const char* fileName, const char* customersFileName, int compressedMode) {
	pMarket->customerCount = 0;
	pMarket->customerArr = NULL;
	pMarket->productCount = 0;
	pMarket->productArr = NULL;
	pMarket->sortOpt = eNone;

	if (compressedMode) {
		if (loadSuperMarketFromFileCompressed(pMarket, fileName, customersFileName)) {
			printf("Supermarket successfully loaded from compressed file\n");
			return 1;
		}
	}
	else {
		if (loadSuperMarketFromFile(pMarket, fileName, customersFileName)) {
			printf("Supermarket successfully loaded from uncompressed file\n");
			return 1;
		}
	}

	pMarket->name = getStrExactLength("Enter market name");
	return pMarket->name != NULL;
}



void printSuperMarket(const SuperMarket* pMarket)
{
	printf("Super Market Name: %s\t", pMarket->name);
	printf("\n");
	printAllProducts(pMarket);
	printf("\n");
	printAllCustomers(pMarket);
}

int addProduct(SuperMarket* pMarket)
{
	char answer;
	char barcode[BARCODE_LENGTH + 1];
	Product* pProd = NULL;

	printf("\n");
	printf("Adding new product? y/Y: ");
	scanf("%c", &answer);
	getchar();

	if (toupper(answer) == 'Y')
	{
		if (!addNewProduct(pMarket))
		{
			free(pProd);
			return 0;
		}
		return 1;
	}
	else if (pMarket->productCount > 0)
	{
		printf("Do you want to increase the amount of an existing product? y/Y: ");
		scanf("%c", &answer);
		getchar();
		if (toupper(answer) == 'Y')
		{
			printAllProducts(pMarket);
			pProd = getProductFromUser(pMarket, barcode);
			if (pProd != NULL) //This barcode exist in stock
				updateProductCount(pProd);
		}
	}
	else
		return 0;
	
	return 1;
}

int addNewProduct(SuperMarket* pMarket)
{
	Product** pTempArr = (Product**)realloc(pMarket->productArr, (pMarket->productCount + 1) * sizeof(Product*));
	if (!pTempArr)
	{
		freeProducts(pMarket);
		return 0;
	}
	pMarket->productArr = pTempArr;
	
	Product* pProd = (Product*)calloc(1, sizeof(Product));
	if (!pProd)
	{
		free(pMarket->productArr);
		return 0;
	}

	initProductNoBarcode(pProd);

	do
	{
		generateBarcode(pProd);
	} while (!isBarcodeUnique(pMarket, pProd->barcode)); //generate new barcode until it is unique
	pMarket->sortOpt = eNone;
	pMarket->productArr[pMarket->productCount] = pProd;
	pMarket->productCount++;

	return 1;
}

int isBarcodeUnique(const SuperMarket* pMarket, const char* barcode)
{
	for (int i = 0; i < pMarket->productCount; i++)
	{
		if (strcmp(pMarket->productArr[i]->barcode, barcode) == 0)
			return 0; // Barcode is not unique
	}
	return 1; // Barcode is unique
}

int isCustomerIdUnique(const SuperMarket* pMarket, const char* id)
{
	for (int i = 0; i < pMarket->customerCount; i++)
	{
		if (strcmp(pMarket->customerArr[i].sId, id) == 0)
		{
			printf("ID %s is not unique\n", id);
			return 0; // ID is not unique
		}
	}
	return 1; // ID is unique
}

int addCustomer(SuperMarket* pMarket)
{
	int isClubMember;
	Customer cust = { 0 };

	do {
		getCustomerID(&cust);
	} while (!isCustomerIdUnique(pMarket, cust.sId));
	
	printf("Is the customer a club member? 1 for yes, 0 for no: ");
	do {
		scanf("%d", &isClubMember);
		if (isClubMember != 0 && isClubMember != 1)
			printf("Invalid input, please enter 1 for yes, 0 for no: ");
	} while (isClubMember != 0 && isClubMember != 1);

	if (isClubMember)
		initClubMemberNoId(&cust);
	else
		initCustomerNoId(&cust);

	pMarket->customerArr = (Customer*)safeRealloc(pMarket->customerArr, (pMarket->customerCount + 1) * sizeof(Customer));
	if (!pMarket->customerArr)
	{
		cust.vTable.deleteObj(&cust);
		return 0;
	}

	pMarket->customerArr[pMarket->customerCount] = cust;
	pMarket->customerCount++;
	return 1;
}


int	doShopping(SuperMarket* pMarket)
{
	Customer* pCustomer = getCustomerShopPay(pMarket);
	if (!pCustomer)
		return 0;

	if (pCustomer->pCart == NULL)
	{
		pCustomer->pCart = (ShoppingCart*)malloc(sizeof(ShoppingCart));
		if (!pCustomer->pCart)
			return 0;
		initCart(pCustomer->pCart);
	}

	fillCart(pMarket, pCustomer->pCart);

	// Check if the cart is empty by checking if the head's next node is NULL
	if (pCustomer->pCart->shoppingItems.head.next == NULL) // did not buy anything
	{
		free(pCustomer->pCart);
		pCustomer->pCart = NULL;
	}

	printf("---------- Shopping ended ----------\n");
	return 1;
}

Customer*	doPrintCart(SuperMarket* pMarket)
{
	Customer* pCustomer = getCustomerShopPay(pMarket);
	if (!pCustomer)
		return NULL;
	if (pCustomer->pCart == NULL)
	{
		printf("Customer cart is empty\n");
		return NULL;
	}
	printCustomerCart(pCustomer);
	
	return pCustomer;
}

int	manageShoppingCart(SuperMarket* pMarket)
{
	Customer* pCustomer = doPrintCart(pMarket);
	char answer;

	if(!pCustomer)
		return 0;

	printf("Do you want to pay for the cart? y/Y, anything else to cancel shopping!\t");
	do {
		scanf("%c", &answer);
	} while (isspace(answer));

	getchar(); //clean the enter

	if (answer == 'y' || answer == 'Y')
		pay(pCustomer);
	else {
		clearCart(pMarket, pCustomer);
		cancelShopping(pCustomer);
	}
	return 1;

}

Customer* getCustomerShopPay(SuperMarket* pMarket)
{
	if (pMarket->customerCount == 0)
	{
		printf("No customer listed to market\n");
		return NULL;
	}

	if (pMarket->productCount == 0)
	{
		printf("No products in market - cannot shop\n");
		return NULL;
	}

	Customer* pCustomer = getCustomerWhoShop(pMarket);
	if (!pCustomer)
	{
		printf("this customer is not listed\n");
		return NULL;
	}

	return pCustomer;
}

void printAllProducts(const SuperMarket* pMarket)
{
	printf("There are %d products\n", pMarket->productCount);
	printf("%-20s %-10s\t", "Name", "Barcode");
	printf("%-20s %-10s %-20s %-15s\n", "Type", "Price", "Count In Stoke", "Expiry Date");
	printf("-------------------------------------------------------------------------------------------------\n");
	
	generalArrayFunction(pMarket->productArr, pMarket->productCount, sizeof(Product**), printProductPtr);
}

void printAllCustomers(const SuperMarket* pMarket)
{
	printf("There are %d listed customers\n", pMarket->customerCount);

	for (int i = 0; i < pMarket->customerCount; i++)
		pMarket->customerArr[i].vTable.print(&pMarket->customerArr[i]);
}


Customer* getCustomerWhoShop(SuperMarket* pMarket)
{
	printAllCustomers(pMarket);
	char searchTerm[MAX_STR_LEN];
	getsStrFixSize(searchTerm, sizeof(searchTerm), "Who is shopping? Enter customer id\n");

	Customer* pCustomer = FindCustomerById(pMarket, searchTerm);
	
	return pCustomer;
}


void fillCart(SuperMarket* pMarket, ShoppingCart* pCart)
{
	printAllProducts(pMarket);
	char op;
	while (1)
	{
		printf("Do you want to shop for a product? y/Y, anything else to exit!!\t");
		do {
			scanf("%c", &op);
		} while (isspace(op));

		getchar(); //clean the enter

		if (op != 'y' && op != 'Y')
			break;

		int count;

		Product* pProd = getProductAndCount(pMarket, &count);

		if(pProd)
		{
			if (!addItemToCart(pCart, pProd->barcode, pProd->price, count))
			{
				printf("Error adding item\n");
				return;
			}
			pProd->count -= count; //item bought!!!
		}
	}
}

void clearCart(SuperMarket* pMarket, Customer* pCustomer)
{
	if (pCustomer->pCart == NULL)
		return;

	NODE* currentNode = pCustomer->pCart->shoppingItems.head.next;

	while (currentNode != NULL)
	{
		ShoppingItem* pItem = (ShoppingItem*)currentNode->key;
		Product* pProd = getProductByBarcode(pMarket, pItem->barcode);
		if (pProd)
			pProd->count += pItem->count; // Return the items to the stock

		currentNode = currentNode->next;
	}
}

Product* getProductAndCount(SuperMarket* pMarket, int* pCount)
{
	char barcode[BARCODE_LENGTH + 1];
	Product* pProd = getProductFromUser(pMarket, barcode);

	if (pProd == NULL)
	{
		printf("No such product\n");
		return NULL;
	} 
	
	if (pProd->count == 0)
	{
		printf("This product is out of stock\n");
		return NULL;
	}
	
	int count;

	do {
		printf("How many items do you want? max %d\n", pProd->count);
		scanf("%d", &count);
	} while (count <= 0 || count > pProd->count);

	*pCount = count;
	return pProd;
}

void printProductByType(SuperMarket* pMarket)
{
	if (pMarket->productCount == 0)
	{
		printf("No products in market\n");
		return;
	}

	eProductType type = getProductType();
	int count = 0;

	for (int i = 0; i < pMarket->productCount; i++)
	{
		if (pMarket->productArr[i]->type == type)
		{
			count++;
			printProduct(pMarket->productArr[i]);
		}
	}

	if (count == 0)
		printf("There are no product of type %s in market %s\n", getProductTypeStr(type), pMarket->name);
}

Product* getProductFromUser(SuperMarket* pMarket, char* barcode)
{
	getBarcodeCode(barcode);
	Product* pProd = getProductByBarcode(pMarket, barcode);

	if (!pProd)
	{
		printf("No such product barcode\n");
		return NULL;
	}

	return pProd;
}

void freeMarket(SuperMarket* pMarket)
{
	free(pMarket->name);
	freeProducts(pMarket);
	freeCustomers(pMarket);
}

void freeProducts(SuperMarket* pMarket)
{
	for (int i = 0; i < pMarket->productCount; i++)
	{
		freeProduct(pMarket->productArr[i]);
		free(pMarket->productArr[i]);
	}
	free(pMarket->productArr);
}

void freeCustomers(SuperMarket* pMarket)
{
	for (int i = 0; i < pMarket->customerCount; i++)
		pMarket->customerArr[i].vTable.deleteObj(&pMarket->customerArr[i]);
	free(pMarket->customerArr);
}

void	getUniquBarcode(char* barcode, SuperMarket* pMarket)
{
	int cont = 1;

	while (cont)
	{
		getBarcodeCode(barcode);
		int index = getProductIndexByBarcode(pMarket, barcode);

		if (index == -1)
			cont = 0;
		else
			printf("This product already in market\n");
	}
}

int getProductIndexByBarcode(SuperMarket* pMarket, const char* barcode)
{
	for (int i = 0; i < pMarket->productCount; i++)
	{
		if (isProduct(pMarket->productArr[i], barcode))
			return i;
	}
	return -1;
}

Product* getProductByBarcode(SuperMarket* pMarket, const char* barcode)
{
	for (int i = 0; i < pMarket->productCount; i++)
	{
		if (isProduct(pMarket->productArr[i], barcode))
			return pMarket->productArr[i];
	}
	return NULL;

}


Customer* FindCustomerById(SuperMarket* pMarket, const char* id)
{
	for (int i = 0; i < pMarket->customerCount; i++)
	{
		if (isCustomerById(&pMarket->customerArr[i], id))
			return &pMarket->customerArr[i];
	}
	return  NULL;
}

eSortOption showSortMenu()
{
	int opt;
	printf("Base on what field do you want to sort?\n");
	do {
		for (int i = 1; i < eNofSortOpt; i++)
			printf("Enter %d for %s\n", i, sortOptStr[i]);
		scanf("%d", &opt);
	} while (opt < 0 || opt >eNofSortOpt);

	return (eSortOption)opt;
}

void* getCompareFunction(eSortOption sort)
{
	int(*compare)(const void* air1, const void* air2) = NULL;

	switch (sort)
	{
	case eName:
		compare = compareProductsByName;
		break;
	case eCount:
		compare = compareProductsByCount;
		break;
	case ePrice:
		compare = compareProductsByPrice;
		break;
	}
	return compare;

}

void sortProducts(SuperMarket* pMarket)
{
	pMarket->sortOpt = showSortMenu();
	int (*compare)(const void* arg1, const void* arg2) = NULL;

	compare = getCompareFunction(pMarket->sortOpt);

	if (compare)
		qsort(pMarket->productArr, pMarket->productCount, sizeof(Product*), compare);
	else
		printf("Error in sorting\n");

}

void findProduct(const SuperMarket* pMarket)
{
	int (*compare)(const void* arg1, const void* arg2) = NULL;
	Product prod = { 0 };
	Product* pProd = &prod;

	compare = getCompareFunction(pMarket->sortOpt);

	switch (pMarket->sortOpt)
	{
	case eName:
		strcpy(pProd->name, getStrExactLength("Enter product name"));
		break;
	case eCount:
		printf("Enter product count\n");
		scanf("%d", &pProd->count);
		break;
	case ePrice:
		printf("Enter product price\n");
		scanf("%f", &pProd->price);
		break;
	case eNone:
		printf("Need to sort products first!!!\n");
		break;
	}

	if (compare)
	{
		Product** pTemp = (Product**)bsearch(&pProd, pMarket->productArr, pMarket->productCount, sizeof(Product*), compare);
		if (!pTemp)
			printf("Product not found\n");
		else
		{
			printf("Product found\n");
			printProduct(*pTemp);
		}
	}
	else {
		printf("The search cannot be performed, array not sorted\n");
	}
}

void handleCustomerStillShoppingAtExit(SuperMarket* pMarket)
{
	Customer* pCust;
	for (int i = 0; i < pMarket->customerCount; i++)
	{
		pCust = &pMarket->customerArr[i];
		if (pCust->pCart)
		{
			printf("Market is closing must pay!!!\n");
			pay(pCust); //will free every thing and update shope info
		}
	}
}

int writeHeader(FILE* fp, const SuperMarket* market) {
	unsigned char headerBytes[2];

	unsigned short header = ((market->productCount & 0x3FF) << 6);
	header |= (strlen(market->name) & 0x3F);
	headerBytes[0] = (header >> 8) & 0xFF;
	headerBytes[1] = header & 0xFF;

	if (fwrite(headerBytes, sizeof(unsigned char), 2, fp) != 2) {
		printf("Error reading header bytes\n");
		return 0;
	}
	if (fwrite(market->name, sizeof(char), strlen(market->name), fp) != strlen(market->name)) {
		printf("Error writing market name\n");
		return 0;
	}
	return 1;
}

int writeProduct(FILE* fp, const Product* pProd) {
	unsigned char prodHead[4] = { 0 };

	unsigned int barcodeValue = 0;
	for (int j = 0; j < 5; j++) {
		barcodeValue |= (pProd->barcode[j + 2] - '0') << (16 - j * 4);
	}

	unsigned int header = (barcodeValue & 0xFFFFF) << 12;
	header |= (pProd->type & 0x3) << 10;
	header |= (strlen(pProd->name) & 0xF) << 6;

	prodHead[0] = (header >> 24) & 0xFF;
	prodHead[1] = (header >> 16) & 0xFF;
	prodHead[2] = (header >> 8) & 0xFF;
	prodHead[3] = header & 0xFF;

	return fwrite(prodHead, sizeof(unsigned char), 4, fp) == 4 &&
		fwrite(pProd->name, sizeof(char), strlen(pProd->name), fp) == strlen(pProd->name) &&
		writePriceAndQuantity(fp, pProd) &&
		writeExpiryDate(fp, pProd);
}

int writePriceAndQuantity(FILE* fp, const Product* pProd) {
	unsigned char priceBytes[3];

	unsigned int priceInfo =
		((pProd->count & 0xFF) << 16) |
		(((int)(pProd->price * 100) % 100 & 0x7F) << 9) |
		((int)pProd->price & 0x1FF);

	priceBytes[0] = (priceInfo >> 16) & 0xFF;
	priceBytes[1] = (priceInfo >> 8) & 0xFF;
	priceBytes[2] = priceInfo & 0xFF;

	return fwrite(priceBytes, sizeof(unsigned char), 3, fp) == 3;
}

int writeExpiryDate(FILE* fp, const Product* pProd) {
	unsigned char dateBytes[2];

	unsigned short dateInfo = ((pProd->expiryDate.day & 0x1F) << 11) |
		((pProd->expiryDate.month & 0xF) << 7) |
		(((pProd->expiryDate.year - 2024) & 0x7) << 4);

	dateBytes[0] = (dateInfo >> 8) & 0xFF;
	dateBytes[1] = dateInfo & 0xFF;

	return fwrite(dateBytes, sizeof(unsigned char), 2, fp) == 2;
}

int saveSuperMarketToFileCompressed(const SuperMarket* pMarket, const char* fileName) {
	CHECK_RETURN_0(pMarket);
	CHECK_MSG_RETURN_0(fileName, "Error: File name is NULL");

	FILE* fp = fopen(fileName, "wb");
	CHECK_MSG_RETURN_0(fp, "Error: Could not open file");

	if (!writeHeader(fp, pMarket)) {
		printf("Couldn't write header\n");
		fclose(fp);
		return 0;
	}

	for (int i = 0; i < pMarket->productCount; i++) {
		if (!writeProduct(fp, pMarket->productArr[i])) {
			CLOSE_RETURN_0(fp);
		}
	}

	fclose(fp);
	return 1;
}


int readHeader(FILE* fp, SuperMarket* pMarket) {
	unsigned char headerBytes[2];

	if (fread(headerBytes, sizeof(unsigned char), 2, fp) != 2) return 0;

	unsigned short header = (headerBytes[0] << 8) | headerBytes[1];

	pMarket->productCount = (header >> 6) & 0x3FF;
	int nameLen = header & 0x3F;

	pMarket->name = (char*)calloc(nameLen + 1, sizeof(char));
	if (!pMarket->name) return 0;

	if (fread(pMarket->name, sizeof(char), nameLen, fp) != nameLen) {
		printf("Error reading supermarket name\n");
		free(pMarket->name);
		return 0;
	}
	return 1;
}

void extractBarcode(unsigned int barcodeNumber, int type, char* barcode) {
	const char* prefix = typePrefix[type];
	barcode[0] = prefix[0];
	barcode[1] = prefix[1];

	for (int j = 0; j < 5; j++) {
		barcode[j + 2] = ((barcodeNumber >> (16 - j * 4)) & 0xF) + '0';
	}

	barcode[7] = '\0';
}

int readProduct(FILE* fp, Product* pProd) {
	unsigned char prodHead[4];

	if (fread(prodHead, sizeof(unsigned char), 4, fp) != 4) {
		printf("Error reading product header\n");
		return 0;
	}

	unsigned int header = (prodHead[0] << 24) | (prodHead[1] << 16) | (prodHead[2] << 8) | prodHead[3];

	unsigned int barcodeNumber = (header >> 12) & 0xFFFFF;
	pProd->type = (header >> 10) & 0x3;

	extractBarcode(barcodeNumber, pProd->type, pProd->barcode);

	int prodNameLen = (header >> 6) & 0xF;

	if (fread(pProd->name, sizeof(char), prodNameLen, fp) != prodNameLen) {
		printf("Error reading product name\n");
		return 0;
	}

	pProd->name[prodNameLen] = '\0';

	return 1;
}

int readPriceAndQuantity(FILE* fp, Product* pProd) {
	unsigned char priceBytes[3];

	if (fread(priceBytes, sizeof(unsigned char), 3, fp) != 3) {
		return 0;
	}

	unsigned int priceInfo = (priceBytes[0] << 16) | (priceBytes[1] << 8) | priceBytes[2];

	pProd->count = (priceInfo >> 16) & 0xFF;
	int fraction = (priceInfo >> 9) & 0x7F;
	int intPart = priceInfo & 0x1FF;
	pProd->price = intPart + (fraction / 100.0);

	return 1;
}

int readExpiryDate(FILE* fp, Product* pProd) {
	unsigned char dateBytes[2];

	if (fread(dateBytes, sizeof(unsigned char), 2, fp) != 2) {
		return 0;
	}

	unsigned short dateInfo = (dateBytes[0] << 8) | dateBytes[1];

	pProd->expiryDate.day = (dateInfo >> 11) & 0x1F;
	pProd->expiryDate.month = (dateInfo >> 7) & 0xF;
	pProd->expiryDate.year = 2024 + ((dateInfo >> 4) & 0x7);

	return 1;
}

int loadSuperMarketFromFileCompressed(SuperMarket* pMarket, const char* fileName, const char* customersFileName) {
	CHECK_RETURN_0(pMarket);
	CHECK_MSG_RETURN_0(fileName, "Error: File name is NULL");

	FILE* fp = fopen(fileName, "rb");
	CHECK_MSG_RETURN_0(fp, "Error: Failed to open file");

	if (!readHeader(fp, pMarket)) {
		printf("Error: Failed to read header\n");
		CLOSE_RETURN_0(fp);
	}

	pMarket->productArr = (Product**)malloc(pMarket->productCount * sizeof(Product*));
	CHECK_MSG_CLOSE_RETURN_0(pMarket->productArr, "Memory allocation failed", fp);

	for (int i = 0; i < pMarket->productCount; i++) {
		if (!(pMarket->productArr[i] = (Product*)malloc(sizeof(Product))) ||
			!readProduct(fp, pMarket->productArr[i]) ||
			!readPriceAndQuantity(fp, pMarket->productArr[i]) ||
			!readExpiryDate(fp, pMarket->productArr[i])) {

			fclose(fp);
			return 0;
		}
	}

	fclose(fp);
	pMarket->customerArr = loadCustomersFromTextFile(customersFileName, &pMarket->customerCount);
	return 1;
}
