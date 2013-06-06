//============================================================================
// Name        : NaiveBayes.cpp
//
//============================================================================

/**************** Check the formula for m-estimate ***********/
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string.h>
#include <cmath>
#include <vector>
#include <map>

#define PI 3.141592654
using namespace std;

vector<string> dataRow;
vector<vector<string> > dataFrameTrain;
vector<vector<string> > dataFrameTest;

vector<vector<string> > kfTrain;
vector<vector<string> > kfTest;

map<string, int> probIdentifiersOK; // for the identifiers vid and pid
map<string, int> probIdentifiersFraud; // for the identifiers vid and pid
map<int, int> probValues;

typedef map<string, int>::iterator stringMapIter;
//Function prototypes
double calculateMean(string,int, vector<vector<string> > );
double calculateSD(string , int , int , vector<vector<string> > );
double calculatePDF(string classifier, int , double , double, int);

int fraudCount = 0, OKCount = 0;


//If we get NA records, we just ignore the records.

void insertRecords()
{	int lineCounter = 0;
	ifstream myfile;
	myfile.open("sales.csv");
	string line;
	char * cptr;

	//insert all the values in a map <int, vector>
	while (getline(myfile, line))
	{
	  int NAFlag = 0,knownLabel = 0;// to populate only known labels in the vector 0 indicates label is known
	  lineCounter++;
	  dataRow.clear();
	  char *c;
	  c = &line[0];
	  cptr = strtok(c,",");
	  dataRow.push_back(cptr);
	  while (cptr != NULL)
	  {
		cptr = strtok(NULL, ",");
		if(cptr != NULL) //&& strcmp(cptr,"NA") != 0)
		{
			if(strcmp(cptr,"NA") != 0){
				dataRow.push_back(cptr);

				 if((strcmp(cptr,"fraud") == 0) || (strcmp(cptr,"ok") == 0))
				 {
					 knownLabel = 0;
				 }
				 /*
				 else{
					 knownLabel = 1;
				 }
	*/
			}
			else
			{
				NAFlag = 1;
			}
		}// if cptr != NULL

		//cout<<dataRow.at(3)<<endl;
	  }// while cptr != null

	  if(NAFlag != 1)
	  {
		  if (knownLabel != 1)
			  dataFrameTrain.push_back(dataRow);
		  else{
			  dataFrameTest.push_back(dataRow);
		  }
	  }

	}
	//cout<<lineCounter<<"----------------"<<endl;
	myfile.close();
	return;
}//insert Records

void enumerateDatasets(vector<vector<string> > dataFrame )
{

	for(unsigned int ii=0; ii < dataFrame.size(); ii++)
	{
	  string label = dataFrame[ii].at(4);
	  string salesID = dataFrame[ii].at(0);
	  string productID = dataFrame[ii].at(1);

	  if (label.compare("ok") == 0)
	  {
		  OKCount++;
		  if(probIdentifiersOK.count(salesID) > 0)
		  {
			  probIdentifiersOK[salesID]++;
		  }
		  else
		  {
			  probIdentifiersOK[salesID] = 1;
		  }
		  if(probIdentifiersOK.count(productID) > 0)
		  {
			  probIdentifiersOK[productID]++;
		  }
		  else
		  {
			  probIdentifiersOK[productID] = 1;
		  }
	  }

	  else if (label.compare("fraud") == 0)
	  {
		  fraudCount++;
		  if(probIdentifiersFraud.count(salesID) > 0)
		  {
			  probIdentifiersFraud[salesID]++;
		  }
		  else
		  {
			  probIdentifiersFraud[salesID] = 1;
		  }
		  if(probIdentifiersFraud.count(productID) > 0)
		  {
			  probIdentifiersFraud[productID]++;
		  }
		  else
		  {
			  probIdentifiersFraud[productID] = 1;
		  }
	  }

	  else if (label.compare("unkn") == 0)
	  {
		  continue;
	  }

	}// end for ii over dataFrame

}//enumerateDatasets

// Classifier tells the label we are looking for computing the mean for
// parameter takes binary values 0 : quantity
//								 1 : value
// Returns -1 on error
double calculateMean(string classifier, int parameter, vector<vector<string> > dataFrame)
{
	int quantSum = 0, valueSum = 0;
	double quantMean, valueMean;
	int quant = 0, value = 0;
	int numCounter = 0;
	for(unsigned int ii=0; ii < dataFrame.size(); ii++)
	{
		  string label = dataFrame[ii].at(4);
		  if (parameter == 0){
			  quant = atoi(dataFrame[ii].at(2).c_str());

			  if(label.compare(classifier) == 0)
			  {
				  quantSum += quant;
				  numCounter++;
			  }

		  }//Parameter == 0

		  else if (parameter == 1){
			  value = atoi(dataFrame[ii].at(3).c_str());

			  if(label.compare(classifier) == 0)
			  {
				  valueSum += value;
				  numCounter++;
			  }

		  }//Parameter == 1
	}// for ii

	if (parameter == 0){
		quantMean = (double)quantSum/numCounter;
		return quantMean;
	}// if param  == 0
	else if(parameter == 1){
		valueMean = (double)valueSum/numCounter;
		return valueMean;
	}//if param == 1
	else
		return -1.0;
}// End calculate mean

//Computes the statistical deviation for given field
//If param == 0, compute quant
//If param == 1, compute value
double calculateSD(string classifier, int parameter, int mean, vector<vector<string> > dataFrame)
{
	double squareSum = 0.0;
	double tempMean = 0.0;
	int quant = 0, value = 0;
	int numCounter = 0;

	for(unsigned int ii=0; ii < dataFrame.size(); ii++)
	{
		  string label = dataFrame[ii].at(4);
		  if (parameter == 0){
			  quant = atoi(dataFrame[ii].at(2).c_str());
			  if(label.compare(classifier) == 0)
			  {
				  squareSum += pow(double(mean - quant),2);
				  numCounter++;
			  }
		  }//Parameter == 0

		  else if (parameter == 1){
			  value = atoi(dataFrame[ii].at(3).c_str());
			  if(label.compare(classifier) == 0)
			  {
				  squareSum += pow(double(mean - value),2);
				  numCounter++;
			  }
		  }//Parameter == 1
	}// for ii

	if (parameter == 0){
		tempMean = (double)squareSum/numCounter-1;
		return sqrt(tempMean);
	}// if param  == 0
	else if(parameter == 1){
		tempMean = (double)squareSum/numCounter-1;
		return sqrt(tempMean);
	}//if param == 1
	else
		return -1;
}

//The value is 1		e^(-(mean-value)^2/(2SD^2))
//			SDxsqrt(2PI)
double calculatePDF(string classifier, int parameter, double mean, double standardDeviation, int unit)
{
	double constant = 1/(standardDeviation * sqrt(2 * PI));
	double pdf = constant * exp(-(((double)mean - unit) * ((double)mean - unit))/(2 * pow(standardDeviation,2)));
	return pdf;
}



int main(int argc, char ** argv){

	double mValue = 5; // For m-estimate when the probability comes out to be equal to zero.
	double pValue = 0.5;

	insertRecords();

	enumerateDatasets(dataFrameTrain);
	double quantMeanOK =  calculateMean("ok", 0, dataFrameTrain);
	double quantMeanFraud =  calculateMean("fraud", 0, dataFrameTrain);
	double valueMeanOK =  calculateMean("ok", 1, dataFrameTrain);
	double valueMeanFraud = calculateMean("fraud", 1, dataFrameTrain);
/*
	cout<<"Mean OK quant "<<quantMeanOK<<" Value "<<valueMeanOK <<endl;
	cout<<"Mean Fraud quant"<<quantMeanFraud<<" Value "<<valueMeanFraud <<endl;
*/
	double quantSDOK = calculateSD("ok",0,quantMeanOK, dataFrameTrain);
	double quantSDFraud = calculateSD("fraud",0,quantMeanFraud, dataFrameTrain);
	double valueSDOK = calculateSD("ok",1,valueMeanOK, dataFrameTrain);
	double valueSDFraud = calculateSD("fraud",1,valueMeanFraud, dataFrameTrain);
/*
	cout<<"SD OK quant "<<quantSDOK<<" Value "<<valueSDOK <<endl;
	cout<<"SD Fraud quant"<<quantSDFraud<<" Value "<<valueSDFraud <<endl;
*/
	string labels[dataFrameTrain.size()];
	double probOK = 0.0,probFraud= 0.0;
	//int truePos = 0, falsePos = 0;
	int totalOKCounter = 0, totalFraudCounter =0;

	int zerocounter = 0;
	for(unsigned int i = 0;i< dataFrameTrain.size(); i++)
	{
		//iterate through labels and predict classes
		string vid = dataFrameTrain[i].at(0), pid = dataFrameTrain[i].at(1);
		int quant = atoi(dataFrameTrain[i].at(2).c_str()), value = atoi(dataFrameTrain[i].at(3).c_str());
		string label = dataFrameTrain[i].at(4);
		//calculate probabilities for each of the labels

		probOK = 	((((double)probIdentifiersOK[vid]/OKCount)+(mValue*pValue))/(OKCount+fraudCount + mValue))*
					((((double)probIdentifiersOK[pid]/OKCount)+(mValue*pValue))/(OKCount+fraudCount + mValue))*
					calculatePDF("ok", 0, quantMeanOK, quantSDOK, quant)*
					calculatePDF("ok", 1, valueMeanOK, valueSDOK, value);

		probFraud = ((((double)probIdentifiersFraud[vid]/fraudCount)+(mValue*pValue))/(OKCount+fraudCount + mValue))*
					((((double)probIdentifiersFraud[pid]/fraudCount)+(mValue*pValue))/(OKCount+fraudCount + mValue))*
					calculatePDF("fraud", 0, quantMeanFraud, quantSDFraud, quant)*
					calculatePDF("fraud", 1, valueMeanFraud, valueSDFraud, value);

		if(probOK == 0 || probFraud ==0)
		{
			zerocounter++;
		}



		if(probOK > probFraud){
			labels[i] = "ok";
			totalOKCounter++;
		}
		else if(probFraud > probOK)
		{
			labels[i] = "fraud";
			totalFraudCounter++;
		}
/*
		if(labels[i].compare(label) == 0){
			truePos++;
		}
		else{
			falsePos++;
		}
*/
		//cout<<labels[i].compare(label)<<endl;
	 }//End for i = 0 through test

		cout<<"OK "<<totalOKCounter << "----"<<" Fraud"<<totalFraudCounter <<endl;

}// main
