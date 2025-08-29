#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "General.h"
#include "FileHelper.h"
#include "SuperFile.h"
#include "Product.h"
#include "myMacros.h"

int saveSuperMarketToFile(const SuperMarket* pMarket, const char* fileName, const char* customersFileName, int compressedMode) {
	CHECK_RETURN_0(pMarket);
	CHECK_MSG_RETURN_0(fileName, "Error: File name is NULL");

	int result = 1;

	if (compressedMode) {
		result = saveSuperMarketToFileCompressed(pMarket, fileName);
	}
	else {
		FILE* fp = fopen(fileName, "wb");
		CHECK_MSG_RETURN_0(fp, "Error opening supermarket file for writing");

		if (!writeStringToFile(pMarket->name, fp, "Error writing supermarket name") ||
			!writeIntToFile(pMarket->productCount, fp, "Error writing product count")) {
			CLOSE_RETURN_0(fp);
		}

		for (int i = 0; i < pMarket->productCount; i++) {
			if (!saveProductToFile(pMarket->productArr[i], fp)) {
				CLOSE_RETURN_0(fp);
			}
		}

		fclose(fp);
	}

	if (!saveCustomersToTextFile(pMarket->customerArr, pMarket->customerCount, customersFileName)) {
		result = 0;
	}

	return result;
}

int	loadSuperMarketFromFile(SuperMarket* pMarket, const char* fileName,
	const char* customersFileName)
{
	FILE* fp;
	fp = fopen(fileName, "rb");
	CHECK_MSG_RETURN_0(fp, "Error: File name is NULL");

	pMarket->name = readStringFromFile(fp, "Error reading supermarket name\n");
	CHECK_MSG_RETURN_0(pMarket->name, "Error: File name is NULL");

	int count;

	if (!readIntFromFile(&count, fp, "Error reading product count")) {
		FREE_CLOSE_FILE_RETURN_0(pMarket->name, fp);
	}

	pMarket->productArr = (Product**)malloc(count * sizeof(Product*));
	if (!pMarket->productArr)
	{
		FREE_CLOSE_FILE_RETURN_0(pMarket->name, fp);
	}

	pMarket->productCount = count;

	for (int i = 0; i < count; i++)
	{
		pMarket->productArr[i] = (Product*)malloc(sizeof(Product));
		if (!pMarket->productArr[i])
		{
			free(pMarket->name);
			CLOSE_RETURN_0(fp);
		}

		if (!loadProductFromFile(pMarket->productArr[i], fp))
		{
			free(pMarket->productArr[i]);
			free(pMarket->name);
			CLOSE_RETURN_0(fp);
		}
	}


	fclose(fp);

	pMarket->customerArr = loadCustomersFromTextFile(customersFileName, &pMarket->customerCount);
	//CHECK_MSG_RETURN_0(pMarket->customerArr, "Error: Failed to load customers");

	return	1;

}

int	saveCustomersToTextFile(const Customer* customerArr, int customerCount, const char* customersFileName)
{
	FILE* fp;

	fp = fopen(customersFileName, "w");
	if (!fp) {
		printf("Error opening customers file to write\n");
		return 0;
	}

	fprintf(fp, "%d\n", customerCount);
	for (int i = 0; i < customerCount; i++)
		customerArr[i].vTable.saveToFile(&customerArr[i], fp);

	fclose(fp);
	return 1;
}

Customer* loadCustomersFromTextFile(const char* customersFileName, int* pCount)
{
	FILE* fp;

	fp = fopen(customersFileName, "r");
	if (!fp) {
		printf("Error open customers file to write\n");
		return NULL;
	}

	Customer* customerArr = NULL;
	int customerCount;

	fscanf(fp, "%d\n", &customerCount);

	if (customerCount > 0)
	{
		customerArr = (Customer*)calloc(customerCount, sizeof(Customer)); //cart will be NULL!!!
		if (!customerArr)
		{
			fclose(fp);
			return NULL;
		}

		for (int i = 0; i < customerCount; i++)
		{
			if (!loadCustomerFromFile(&customerArr[i], fp))
			{
				freeCustomerCloseFile(customerArr, i, fp);
				return NULL;
			}
		}
	}

	fclose(fp);
	*pCount = customerCount;
	return customerArr;
}


void freeCustomerCloseFile(Customer* customerArr, int count, FILE* fp)
{
	for (int i = 0; i < count; i++)
	{
		free(customerArr[i].name);
		customerArr[i].name = NULL;
		if (customerArr[i].pDerivedObj)
		{
			free(customerArr[i].pDerivedObj);
			customerArr[i].pDerivedObj = NULL;
		}
	}
	free(customerArr);
	fclose(fp);
}