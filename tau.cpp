#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

#define VAR_NUMBER_LINE 1
#define END_CHAR ';'
#define POS_TERM_ENTRY "01"
#define NEG_TERM_ENTRY "10"
#define DC_TERM_ENTRY "11"
#define START_VAR_CHAR 'a'
#define MAX_BINATE_NUM 9999

//Number of variables in expression
int numVars;

//Class representing cube
class cube
{
	public:
		cube()
		{
			entries.clear(); 
		}
		
		string getEntry(int index)
		{
			if(index<0 || index>(entries.size()-1))
			{
				cout << "Invalid index: " << index << endl;
				return "";
			}
			else
			{
				return entries[index];
			}
		}
		
		void allocateEntries(int numEntries)
		{
			if(numEntries<1 )
			{
				cout << "Invalid number of entries to allocate: " << numEntries << endl;
				return;
			}
			else
			{
				entries.clear();
				for(int i=0; i <numEntries; i++)
				{
					entries.push_back("11");
				}
			}
		}
		
		void setEntry(int index, string entry)
		{
			if(index<0 || index>(numVars-1))
			{
				cout << "Invalid index: " << index << endl;
				return;
			}
			else
			{
				if(entry != "01" && entry != "10" && entry != "11")
				{
					cout << "Invalid entry: " << entry << endl;
					return;
				}
				entries[index] = entry;
			}
		}
		
		bool isAllDCCube()
		{
			for(int i=0; i< entries.size(); i++)
			{
				if(entries[i] != DC_TERM_ENTRY)
				{
					return false;
				}
			}
			return true;
		}
		
		bool isSingleVarCube()
		{
			int numNonDC = 0;
			for(int i=0; i< entries.size(); i++)
			{
				if(entries[i] != DC_TERM_ENTRY)
				{
					numNonDC++;
				}
			}
			if(numNonDC==1)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		
		int singleVarCubePolarity()
		{
			if(isSingleVarCube())
			{
				for(int i=0; i< entries.size(); i++)
				{
					if(entries[i] != DC_TERM_ENTRY)
					{
						if(entries[i]==POS_TERM_ENTRY)
						{
							return 1;
						}
						else
						{
							return -1;
						}
					}
				}
			}
			else
			{
				//Not a single var cube
				return 0;
			}
		}
		
		int singleVarCubeEntryIndex()
		{
			if(isSingleVarCube())
			{
				for(int i=0; i< entries.size(); i++)
				{
					if(entries[i] != DC_TERM_ENTRY)
					{
						return i;
					}
				}
			}
			else
			{
				//Not a single var cube
				return -1;
			}
		}
		
		
		
	private:
		vector<string> entries;
};

vector<cube> cubeList;

//Read the input file
void parseFile(char * fileName);
//Parse single line
void parseLine(string line, int lineNumber);
//Get a entry (01,10,11) from char (+ , - )
string getEntryFromChar(char charSign);
//Get SOP term string
string getSOPTerm(cube c);
//Get whole SOP expression
string getSOPExpression(vector<cube> cl);
//get if unate or not
bool isUnate(vector<cube> cl);
//Get if specific variable is unate
bool isUnateWrt(vector<cube> cl, int varIndex);
//Get binate number (|true - compl|) for specific variable
int binateNumber(vector<cube> cl, int varIndex);
//Get pos cofactor wrt specific variable
vector<cube> posCofactor(vector<cube> cl, int varIndex);
//Get neg cofactor wrt specific variable
vector<cube> negCofactor(vector<cube> cl, int varIndex);
//Is cubelist a tautology?
bool isTautology(vector<cube> cl);
//Get most binate variable index
int mostBinate(vector<cube> cl);
//Has all DC cube?
bool hasAllDCCube(vector<cube> cl);
//Single cube in list with both polarities?
bool hasSingleCubeBothPolarities(vector<cube> cl);

int main(int argc, char* argv[])
{
  if(argc <2)
  {
	  cout << "Please provide a file to read from." <<endl;
	  return -1;
  }
  else
  {
	 parseFile(argv[1]);
  }
  
  //Print the SOP expression read in
  cout << "Initial expression: " << getSOPExpression(cubeList) << endl;
  
  
  if(isTautology(cubeList))
  {
	  cout << getSOPExpression(cubeList) << " is a tautology.";
  }
  else
  {
	  cout << getSOPExpression(cubeList) << " is not a tautology.";
  }
  cout << endl;
}

bool isTautology(vector<cube> cl)
{
	if(isUnate(cl))
	{
		//Apply unate tautology termination rules directly (rule 1 and 2)
		//A unate cube-list for function f is a tautology iff
		//it contains the all don’t care cube = [11 11 11 ... 11]
		//Has all DC cube = yes
		//Missing DC cube = false
		cout << "Expression: " << getSOPExpression(cl) << " is unate" << endl;
		if(hasAllDCCube(cl))
		{
			cout << "...and falls under the (stuff + 1 + stuff == 1) rule: is tautology" << endl;
			return true;
		}
		else
		{
			cout << "...and falls under the missing all DC cube rule: not tautology" << endl;
			return false;
		}
	}
	else if(hasSingleCubeBothPolarities(cl))
	{
		cout << "Expression: " << getSOPExpression(cl) << " has a single variable SOP terms that appear in both polarities: is tautology." << endl;
		return true;
	}
	else
	{
		int mostBinateIndex = mostBinate(cl);
		cout << "Splitting " << getSOPExpression(cl) << " into cofactors with respect to: " << (char)('a'+mostBinateIndex) << endl;
		vector<cube> fx = posCofactor(cl,mostBinateIndex);
		vector<cube> fxbar = negCofactor(cl,mostBinateIndex);
		cout << "f" << (char)('a'+mostBinateIndex) << ": " << getSOPExpression(fx) << endl;
		cout << "f" << (char)('a'+mostBinateIndex) << "': "<< getSOPExpression(fxbar) << endl;
		return isTautology(fx) && isTautology(fxbar);
	}
}

bool hasSingleCubeBothPolarities(vector<cube> cl)
{
	//Maintain list of polarity found
	vector<int> polarityFound;
	//Allocate space with 0's (no polarity found)
	for(int i =0; i< numVars; i++)
	{
		polarityFound.push_back(0);
	}
	int tmpIndex = 0;
	int tmpPolarity = 0;
	//Loop through each cube in list
	for(int cubeIndex=0; cubeIndex< cl.size(); cubeIndex++)
	{
		if(cl[cubeIndex].isSingleVarCube())
		{
			//Single var cube found
			tmpIndex = cl[cubeIndex].singleVarCubeEntryIndex();
			tmpPolarity = cl[cubeIndex].singleVarCubePolarity();
			if(polarityFound[tmpIndex] == 0)
			{
				//Not previously found yet
				polarityFound[tmpIndex] = tmpPolarity;
			}
			else
			{
				if( polarityFound[tmpIndex] != tmpPolarity )
				{
					//Appears again as single cube with opposite polarity
					return true;
				}
				else
				{
					polarityFound[tmpIndex] = tmpPolarity;
				}
			}
		}
	}
	return false;
	
}

bool hasAllDCCube(vector<cube> cl)
{
	for(int cubeIndex=0; cubeIndex< cl.size(); cubeIndex++)
	{
		if(!(cl[cubeIndex].isAllDCCube()))
		{
			return false;
		}
	}
	return true;	
}

int mostBinate(vector<cube> cl)
{
	int minBinateNum = MAX_BINATE_NUM;
	int tmp = 0;
	int binateVar = 0;
	for(int varIndex=0; varIndex< numVars; varIndex++)
	{
		tmp = binateNumber(cl,varIndex);
		if(tmp < minBinateNum)
		{
			minBinateNum = tmp;
			binateVar = varIndex;
		}
	}
	return binateVar;
}


bool isUnate(vector<cube> cl)
{	
	//Loop through each cube
	for(int i=0; i < numVars; i++)
	{
		//Check if unate wrt each variable
		if(!isUnateWrt(cl,i) )
		{
			return false;
		}
	}
	return true;
}

bool isUnateWrt(vector<cube> cl, int varIndex)
{
	//Loop through each cube in list
	//Must be all true or all false
	int numTrue = 0;
	int numFalse = 0;
	int numDC = 0;
	for(int cubeIndex=0; cubeIndex< cl.size(); cubeIndex++)
	{
		if(cl[cubeIndex].getEntry(varIndex) == POS_TERM_ENTRY)
		{
			numTrue++;
		}
		else if(cl[cubeIndex].getEntry(varIndex) == NEG_TERM_ENTRY)
		{
			numFalse++;
		}
		else if(cl[cubeIndex].getEntry(varIndex) == DC_TERM_ENTRY)
		{
			numDC++;
		}
		else
		{
			cout << "Invalid entry " << cl[cubeIndex].getEntry(varIndex) << " in cube " << cubeIndex << " index " << varIndex << endl;
		}
	}
	
	if( (numTrue>0 && numFalse==0) || (numTrue==0 && numFalse>0) )
	{
		return true;
	}
	else if(numTrue==0 && numFalse==0)
	{
		//All DCs found
		//Considered unate in that variable
		return true;
	}
	else
	{
		return false;
	}
}

int binateNumber(vector<cube> cl, int varIndex)
{
	//Very similar to wrt unate check
	//Loop through each cube in list
	int numTrue = 0;
	int numFalse = 0;
	int numDC = 0;
	for(int cubeIndex=0; cubeIndex< cl.size(); cubeIndex++)
	{
		if(cl[cubeIndex].getEntry(varIndex) == POS_TERM_ENTRY)
		{
			numTrue++;
		}
		else if(cl[cubeIndex].getEntry(varIndex) == NEG_TERM_ENTRY)
		{
			numFalse++;
		}
		else if(cl[cubeIndex].getEntry(varIndex) == DC_TERM_ENTRY)
		{
			numDC++;
		}
		else
		{
			cout << "Invalid entry " << cl[cubeIndex].getEntry(varIndex) << " in cube " << cubeIndex << " index " << varIndex << endl;
		}
	}
	//If num true and false are both zero then it is a DC
	//DC is not binate so return a huge number so not to be confused for a very binate variable
	if(numTrue==0 && numFalse==0)
	{
		return MAX_BINATE_NUM;
	}
	else
	{
		return abs(numTrue-numFalse);
	}
}

vector<cube> posCofactor(vector<cube> cl, int varIndex)
{
	//cl is copy of list - work on that and return that
	//Loop through each cube in list
	vector<int> toErase;
	for(int cubeIndex=0; cubeIndex< cl.size(); cubeIndex++)
	{
		//cout << "Cube index " << cubeIndex << " varIndex " << varIndex << " = " << cl[cubeIndex].getEntry(varIndex) << endl;
		if(cl[cubeIndex].getEntry(varIndex) == POS_TERM_ENTRY)
		{
			//make this slot 11 == don’t care
			cl[cubeIndex].setEntry(varIndex,DC_TERM_ENTRY);
		}
		else if(cl[cubeIndex].getEntry(varIndex) == NEG_TERM_ENTRY)
		{
			//remove this cube from list
			cl.erase(cl.begin() + cubeIndex);
			cubeIndex--;
		}
		else if(cl[cubeIndex].getEntry(varIndex) == DC_TERM_ENTRY)
		{
			//Leave alone
		}
		else
		{
			cout << "Invalid entry " << cl[cubeIndex].getEntry(varIndex) << " in cube " << cubeIndex << " index " << varIndex << endl;
		}
	}
	//Erase by not placing 
	
	//Erase after iteration
	for(int i=0;i<toErase.size();i++)
	{
		cl.erase(cl.begin() + toErase[i]);
	}
	return cl;
}

vector<cube> negCofactor(vector<cube> cl, int varIndex)
{
	//cl is copy of list - work on that and return that
	//Loop through each cube in list
	for(int cubeIndex=0; cubeIndex< cl.size(); cubeIndex++)
	{
		if(cl[cubeIndex].getEntry(varIndex) == POS_TERM_ENTRY)
		{
			//remove this cube from list
			cl.erase(cl.begin() + cubeIndex);
			cubeIndex--;
		}
		else if(cl[cubeIndex].getEntry(varIndex) == NEG_TERM_ENTRY)
		{
			//make this slot 11 == don’t care
			cl[cubeIndex].setEntry(varIndex,DC_TERM_ENTRY);
		}
		else if(cl[cubeIndex].getEntry(varIndex) == DC_TERM_ENTRY)
		{
			//Leave alone
		}
		else
		{
			cout << "Invalid entry " << cl[cubeIndex].getEntry(varIndex) << " in cube " << cubeIndex << " index " << varIndex << endl;
		}
	}
	return cl;
}

string getSOPTerm(cube c)
{
	string rv = "";
	bool isNegated = false;
	for(int i =0; i< numVars; i++)
	{
		char tmpChar;
		isNegated = false;
		if(c.getEntry(i) == POS_TERM_ENTRY || c.getEntry(i) == NEG_TERM_ENTRY)
		{
			tmpChar = START_VAR_CHAR + i;
			if(c.getEntry(i) == NEG_TERM_ENTRY)
			{
				isNegated = true;
			}
			rv+=tmpChar;
			if(isNegated)
			{
				rv+='\'';
			}
		}
	}
	return rv;
}

string getSOPExpression(vector<cube> cl)
{
	string rv;
	for(int i =0; i<cl.size(); i++)
	{
		rv+=getSOPTerm(cl[i]);
		if(i!=cl.size()-1)
		{
			rv+=" + ";
		}
	}
	return rv;
}


void parseFile(char * fileName)
{
	string line;
	ifstream infile(fileName);
	int lineCount = 0;
	while (getline(infile, line))
	{
		lineCount++;
		//cout << "Parsing input line: " << line << endl;
		parseLine(line, lineCount);
	}
	if(lineCount == 0)
	{
		cout << "No such file or empty file." << endl;
		exit(-1);
	}
}
void parseLine(string line, int lineNumber)
{
	if(lineNumber == VAR_NUMBER_LINE)
	{
		//Read in the number of variables
		numVars = atoi(line.c_str());
		//cout << "Number of variables = " << numVars << endl;
		//Variable names are
		//cout << "Variable names are: ";
		for(int i=0; i <numVars; i++)
		{
			//cout << (char)(START_VAR_CHAR+i) << " ";
		}
		//cout << endl;
	}
	else
	{
		if(line[0] == END_CHAR)
		{
			//First char is semicolon - the end
			//cout << "End of file." << endl;
		}
		else
		{
			//Must be a cube desciption
			//Read char by char
			//Chars are in groups of 4
			//0123012301230
			//+ a + b + c ;
			//To get index from char, use ascii
			//index = char - START_VAR_CHAR (a -> 0, b->1 ...etc.
			//Allocate a new cube
			cube * tmpCube = new cube();
			//Allocate number of variables in cube
			tmpCube->allocateEntries(numVars); 
			
			string tmpEntry;
			int tmpIndex = 0;
			//Begin reading line, jump by 4
			for(int i=0; i <line.length(); i+=4)
			{
				if(line[i] != END_CHAR)
				{
					tmpEntry = getEntryFromChar(line[i]);
					tmpIndex = line[i+2] - START_VAR_CHAR;
					//cout << line[i+2] << " found. Entry " << tmpEntry << " placed in index " << tmpIndex << " of the cube." << endl;
					tmpCube->setEntry(tmpIndex,tmpEntry);
				}
			}
			//Done reading cube line
			//Copy cube object into cubelist
			cubeList.push_back(*tmpCube);
			//Deallocate old object
			delete(tmpCube);
		}
	}
}

string getEntryFromChar(char charSign)
{
	if(charSign == '+')
	{
		return POS_TERM_ENTRY;
	}
	else
	{
		// -
		return NEG_TERM_ENTRY;
	}
}
