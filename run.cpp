#include <iostream>
#include <cstdlib>

void runBatchFile()
{
	const char * command = "cmd.exe /C setup_env.bat";
	int result = std::system(command);
	
	if (result == 0) 
	{
		std::cout << "Batch file executed successfully\n";
	} 
	else 
	{
		std::cout << "Error executing file. " << result << "\n";
	}

	const char * runCommand = "cmd.exe /C run.bat";
	int runResult = std::system(runCommand);
	
	if (runResult == 0) 
	{
		std::cout << "Batch file executed successfully\n";
	} 
	else 
	{
		std::cout << "Error executing file. " << runResult << "\n";
	}
}

int main()
{
	runBatchFile();
	return 0;
}
