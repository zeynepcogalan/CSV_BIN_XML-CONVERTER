#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <math.h>
#include <libxml/xmlschemastypes.h>

// Struct that holds the customer information we get from the csv file
// We made the size of the char arrays according to the xsd constraints
// we determined the sizes of the non-restrictions ourselves
typedef struct
{
    char name[20];
    char surname[30];
    char gender;
    char occupancy[30];
    char level_of_education[4];
    char email[50];
    char bank_account_number[13]; /*4dig+"-"+7 dig*/
    char IBAN[28];
    char account_type[14];
    char currency_unit[4];
    int total_balance_available;
    char available_for_loan[8];

} Customer;

// We do the writing to the binary file in this function.
// we return true false to see if the file can be written
// As a parameter to this function: we get the address of the file we will write in,
// the address of the data we will write to this file, and the number of these data.
bool write_data(char *file_name, Customer *data, int total)
{
    FILE *file;

    file = fopen(file_name, "wb");

    /*if there is a problem opening the file*/
    if (file == NULL)
    {
        return false;
    }

    // We compare the function with a value of 1 and expect it to return 1 byte as a result.
    //  If it doesn't, we return an error.
    if (fwrite(&total, sizeof(int), 1, file) != 1)
    {
        return false;
    }

    // We take the "total" pieces things, each of which is "customer" in size,
    //  from the "data" address and write it to the "file". we expect it to return "total" pieces bytes as a result.
    if (fwrite(data, sizeof(Customer), total, file) != total)
    {
        return false;
    }
    //we check if the file is closed
    if (fclose(file) == EOF)
    {
        return false;
    }

    return true;
}


//we read the binary file with this function
//We take the name of the file to be read and the total as parameters. total will hold the total number of customers. 
//We throw these customers into the customer malloc and return a customer
Customer *read_data(char *file_name, int *total)
{

    FILE *file;
    file = fopen(file_name, "rb");

    if (file == NULL) /*if we had trouble reading the file*/
    {
        return NULL;
    }

    /* We read the size of 1 thing from the file and store it in total and expect the result to be 1 */
    if (fread(total, sizeof(int), 1, file) != 1)
    {
        return NULL;
    }

    /* the number of struct attributes times the number of structs for the malloc capacity in heap */
    Customer *data = malloc(sizeof(Customer) * (*total));

    if (fread(data, sizeof(Customer), *total, file) != *total)
    {
        free(data);
        return NULL;
    }

    if (fclose(file) == EOF)
    {
        //if file is not read we free memory
        free(data);
        return NULL;
    }

    return data;
}

/*
(value & 0x000000ff) << 24; // move byte 0 to byte 3
(value & 0x0000ff00) << 8;  // move byte 1 to byte 2
(value & 0x00ff0000) >> 8;  // move byte 2 to byte 1
(value & 0xff000000) >> 24; // move byte 3 to byte 0
*/
unsigned int litToBig_endian(int num)
{
    return (((num >> 24) & 0x000000ff) | ((num >> 8) & 0x0000ff00) | ((num << 8) & 0x00ff0000) | ((num << 24) & 0xff000000));
}

int main(void)
{
    xmlDocPtr doc = NULL;
    xmlNodePtr root_node = NULL;
    char temp[256];//we will use it to write xmle
    Customer *customers;

    //*we keep the customer information we read here, the first index is empty*/
    customers = malloc(sizeof(Customer) * 100);

    char inputFile[50], outputFile[50], myConverter[50];
    int typeNum;

    printf("Please enter input file name , output file name and type number: \n");
    printf("1=CSV to BIN, 2=BIN to XML, 3=XML validation with XSD\n");
    scanf("%s %s %s %d", myConverter, inputFile, outputFile, &typeNum);

     //if we choose csv to bin
    if (typeNum == 1)
    {

        FILE *file;

        file = fopen(inputFile, "r");

        if (file == NULL)
        {
            printf("error opening file.\n");
            return 1;
        }


        int records = 0; /*It keeps the number of customers we registered*/

       
        do
        {
            /*because we don't read the first line of the csv*/
            if (records == 0)
            {
                char format_name[200];
                char comma;
                /*Arrays don't have ampersant(&) at the beginning because they act like pointers*/
                /*read the format names in the first line and pass the file to the second line*/
                fscanf(file, "%s,\n", format_name);
                fscanf(file, "%c\n", &comma);
            }

            //Here we assign the data separated by commas to the customer. 
            //we assign the comma to the comma char
            if (records != 0)
            {
                char comma;
                fscanf(file, "%21[^,]\n", customers[records].name);
                fscanf(file, "%c\n", &comma);
                fscanf(file, "%31[^,]\n", customers[records].surname);
                fscanf(file, "%c\n", &comma);
                fscanf(file, "%c\n", &customers[records].gender);
                fscanf(file, "%c\n", &comma);
                fscanf(file, "%31[^,]\n", customers[records].occupancy);
                fscanf(file, "%c\n", &comma);
                fscanf(file, "%4[^,]\n", customers[records].level_of_education);
                fscanf(file, "%c\n", &comma);
                fscanf(file, "%51[^,]\n", customers[records].email);
                fscanf(file, "%c\n", &comma);
                fscanf(file, "%13[^,]\n", customers[records].bank_account_number);
                fscanf(file, "%c\n", &comma);
                fscanf(file, "%28[^,]\n", customers[records].IBAN);
                fscanf(file, "%c\n", &comma);
                fscanf(file, "%14[^,]\n", customers[records].account_type);
                fscanf(file, "%c\n", &comma);
                fscanf(file, "%5[^,]\n", customers[records].currency_unit);
                fscanf(file, "%c\n", &comma);
                fscanf(file, "%d\n", &customers[records].total_balance_available);
                fscanf(file, "%c\n", &comma);
                fscanf(file, "%s\n", customers[records].available_for_loan);
            }
            records++;

            if (ferror(file))
            {
                printf("Error reading file.\n");
                return 1;
            }

        } while (!feof(file)); // iterate until file is empty.

        fclose(file);
        //We write what we read from csv to binary
        if (write_data(outputFile, customers, records))
            printf("CSV file is read and data are written in binary file\n");
    }

    //if we choose bin to xml
    else if (typeNum == 2)
    {
        free(customers); // to free memory space that we take with malloc method

        int total = 0;
        Customer *file_data;

        file_data = read_data(inputFile, &total);

        if (file_data == NULL)
        {
            printf("Error reading from file.\n");
            return 1;
        }

        doc = xmlNewDoc(BAD_CAST "1.0");                  // to helps to keep data in tree form
        root_node = xmlNewNode(NULL, BAD_CAST "records"); // to creat a new node
        xmlDocSetRootElement(doc, root_node);             // to assign root

        /*
        xmlNewChild(xmlNodePtr parent, xmlNsPtr ns, const xmlChar *name, const xmlChar *content)
        -->  adds a new child to wanted node and returns a xmlNodePtr type pointer.
        xmlNewProp(xmlNodePtr node, const xmlChar *name, const xmlChar *value)
        -->  adds a new attribute to an element
        */
        for (int i = 1; i < total; i++)
        {
            sprintf(temp, "%d", i);
            xmlNodePtr nodeId = xmlNewChild(root_node, NULL, BAD_CAST "row", NULL);
            xmlNewProp(nodeId, BAD_CAST "id", BAD_CAST temp);

            xmlNodePtr nodeCusInfo = xmlNewChild(nodeId, NULL, BAD_CAST "Customer_info", NULL);
            xmlNodePtr nodeBankInfo = xmlNewChild(nodeId, NULL, BAD_CAST "Bank_Account_info", NULL);

            sprintf(temp, "%s", file_data[i].name);
            xmlNewChild(nodeCusInfo, NULL, BAD_CAST "name", BAD_CAST temp);

            sprintf(temp, "%s", file_data[i].surname);
            xmlNewChild(nodeCusInfo, NULL, BAD_CAST "surname", BAD_CAST temp);

            sprintf(temp, "%c", file_data[i].gender);
            xmlNewChild(nodeCusInfo, NULL, BAD_CAST "gender", BAD_CAST temp);

            sprintf(temp, "%s", file_data[i].occupancy);
            xmlNewChild(nodeCusInfo, NULL, BAD_CAST "occupancy", BAD_CAST temp);

            sprintf(temp, "%s", file_data[i].level_of_education);
            xmlNewChild(nodeCusInfo, NULL, BAD_CAST "level_of_education", BAD_CAST temp);

            sprintf(temp, "%s", file_data[i].email);
            xmlNewChild(nodeCusInfo, NULL, BAD_CAST "email", BAD_CAST temp);

            sprintf(temp, "%s", file_data[i].bank_account_number);
            xmlNewChild(nodeBankInfo, NULL, BAD_CAST "bank_account_number", BAD_CAST temp);

            sprintf(temp, "%s", file_data[i].IBAN);
            xmlNewChild(nodeBankInfo, NULL, BAD_CAST "IBAN", BAD_CAST temp);

            sprintf(temp, "%s", file_data[i].account_type);
            xmlNewChild(nodeBankInfo, NULL, BAD_CAST "account_type", BAD_CAST temp);

            sprintf(temp, "%d", file_data[i].total_balance_available);
            xmlNodePtr totBalanceAvailable = xmlNewChild(nodeBankInfo, NULL, BAD_CAST "total_balance_available", BAD_CAST temp);

            sprintf(temp, "%s", file_data[i].currency_unit);
            xmlNewProp(totBalanceAvailable, BAD_CAST "currency_unit ", BAD_CAST temp);

            int a = litToBig_endian(file_data[i].total_balance_available);
            sprintf(temp, "%d", a); // to convert char
            xmlNewProp(totBalanceAvailable, BAD_CAST "bigEnd_version ", BAD_CAST temp);

            sprintf(temp, "%s", file_data[i].available_for_loan);
            xmlNewChild(nodeBankInfo, NULL, BAD_CAST "available_for_loan", BAD_CAST temp);
        }

        xmlSaveFormatFileEnc(outputFile, doc, "UTF-8", 1); // for saving xml in a file
        xmlSaveFormatFileEnc("-", doc, "UTF-8", 1);  //for printing xml on console
        xmlFreeDoc(doc);
        xmlCleanupParser();
        free(file_data);
        return 0;
    }
    else if (typeNum == 3)
    {

        xmlDocPtr doc;
        xmlSchemaPtr schema = NULL;
        xmlSchemaParserCtxtPtr ctxt;

        char *XMLFileName = inputFile;  // write your xml file here
        char *XSDFileName = outputFile; // write your xsd file here

        xmlLineNumbersDefault(1);                   // set line numbers, 0> no substitution, 1>substitution
        ctxt = xmlSchemaNewParserCtxt(XSDFileName); // create an xml schemas parse context
        schema = xmlSchemaParse(ctxt);              // parse a schema definition resource and build an internal XML schema
        xmlSchemaFreeParserCtxt(ctxt);              // free the resources associated to the schema parser context

        doc = xmlReadFile(XMLFileName, NULL, 0); // parse an XML file
        if (doc == NULL)
        {
            fprintf(stderr, "Could not parse %s\n", XMLFileName);
        }
        else
        {
            xmlSchemaValidCtxtPtr ctxt; // structure xmlSchemaValidCtxt, not public by API
            int ret;

            ctxt = xmlSchemaNewValidCtxt(schema);  // create an xml schemas validation context
            ret = xmlSchemaValidateDoc(ctxt, doc); // validate a document tree in memory
            if (ret == 0)                          // validated
            {
                printf("%s validates\n", XMLFileName);
            }
            else if (ret > 0) // positive error code number
            {
                printf("%s fails to validate\n", XMLFileName);
            }
            else // internal or API error
            {
                printf("%s validation generated an internal error\n", XMLFileName);
            }
            xmlSchemaFreeValidCtxt(ctxt); // free the resources associated to the schema validation context
            xmlFreeDoc(doc);
        }
        // free the resource
        if (schema != NULL)
            xmlSchemaFree(schema); // deallocate a schema structure

        xmlSchemaCleanupTypes(); // cleanup the default xml schemas types library
        xmlCleanupParser();      // cleans memory allocated by the library itself
        xmlMemoryDump();         // memory dump
        return (0);
    }

    else
    {
        printf("Error writing to file.\n");
        return 1;
    }
}
