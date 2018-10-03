
#include "hooks.hpp"

Hooks::Hooks(bool hook_format, string filename)
{
	if(hook_format)
	{
		fast_Compile_Hooks = true;
	}
	else
	{
		indiv_Compile_Hooks = true;
	}
	if(filename.empty())
	{
		cout<<fg::red<<"No output filename passed in to Hooks::Hooks() constructor."<<endl;
		cin.get();
		exit(1);
	}
	else
	{
		filename_out = filename;
	}
}

void Hooks::align_hook(int align_sizeL ,string filename, string command)
{
	filename = rem_extension(filename);
	filename.append(".o");
	gpp_link(filename, "make hook_gpp_link align_size=" + to_string(align_sizeL));
}

void Hooks::apply_Hook(string current_file, int offset)
{
	int align_sizeL;
	char *hook_F = fReadBinaryFile(current_file);
	int Bytes_to_write = get_bytes(current_file);
	while(Bytes_to_write == false) //in case the hook is bigger then supposable allocate more memory. 
	{
		align_sizeL = get_bytes(current_file, Bytes_to_write) * 2;
		align_hook(align_sizeL, current_file, "make hook_gpp_link PRIME_NAME=");
		Bytes_to_write = get_bytes(current_file);	
	}
	cout<<fg::magenta<<"APPLY HOOK : "<<current_file <<"    Number of instructions: "<<Bytes_to_write<<fg::reset<<endl;
	fWriteBinaryFile(filename_out, hook_F, offset, Bytes_to_write);
	cout<<"\n";
}

void Hooks::parse_build(int offset, string alone_Filename)
{
	boost::filesystem::path p("./build");
	boost::filesystem::directory_iterator end_itr;
	alone_Filename = rem_extension(alone_Filename);
	string current_file;
	for (boost::filesystem::directory_iterator itr(p); itr != end_itr; ++itr)
    {
		if (boost::filesystem::is_regular_file(itr->path())) 
		{
            current_file = itr->path().string();
			if(boost::filesystem::extension(current_file).compare(".o")==0)
			{
				if(current_file.find(alone_Filename)!=string::npos)
				{
					if(!gpp_link(current_file, "make hook_gpp_link"))
					{
						cout<<fg::red<<"gpp_link failed."<<endl;
						cin.get();
						exit(1);
					}
					break;
				}	
			}
		}
	}
	for (boost::filesystem::directory_iterator itr(p); itr != end_itr; ++itr)
    {
		if (boost::filesystem::is_regular_file(itr->path())) 
		{
            current_file = itr->path().string();
			if(boost::filesystem::extension(current_file).compare(".bin")==0)
			{
				if(current_file.find(alone_Filename)!=string::npos)
				{
					apply_Hook(current_file, offset);
					break;
				}
			}
		}
	}
}

void Hooks::build_O(string current_file, string Final_Filename, string alone_Filename)
{
	cout<<fg::cyan<<"BUILDING .O FILES --------------------------------->"<<fg::reset<<endl;
	string command = "make _hooks OBJ_NAME_=";
	command.append(Final_Filename);
	alone_Filename.insert(0, " OBJS=");
	command.append(alone_Filename);
	cout<<"\n";
	if(system(&command[0]))
	{
		cout<<fg::red<<"Error when calling "<<command<<endl;
		cin.get();
		exit(1);
	}
	cout<<"\n";	
}

int Hooks::compile_Hook(string current_file, string Final_Filename, string alone_Filename)
{
	int offset = parse_offset(current_file, "ROffset = ");
	cout<<hex<<"ROffset = "<<offset<<dec<<endl;
	if(fast_Compile_Hooks)
	{
		if(system("make _fast_hooks"))
		{
			cout<<fg::red<<"Error when calling '_fast_hooks' "<<endl;
			cin.get();
			exit(1);
		}
		fast_Compile_Hooks = false;
	}
	else if (indiv_Compile_Hooks)
	{	
		build_O(current_file, Final_Filename, alone_Filename);
	}
	parse_build(offset, alone_Filename);
	return 1;
}

void Hooks::parse_hooks()
{
	boost::filesystem::path p("\hooks");
	boost::filesystem::directory_iterator end_itr;
	cout<<"\n";
	cout<<fg::cyan<<"Available hooks : "<<fg::reset<<endl;
	cout<<"\n";
	for (boost::filesystem::directory_iterator itr(p); itr != end_itr; ++itr)
    {
		if (boost::filesystem::is_regular_file(itr->path())) 
		{
            string current_file = itr->path().string();
			size_t pos = current_file.find("hook_");
			if (pos!=string::npos)
			{
				string end = current_file.substr (pos);
				string Final_Filename;
				Final_Filename.append(end);
				string alone_Filename = Final_Filename;
				cout<<style::bold<<Final_Filename<<style::reset<<endl;
				Final_Filename = rem_extension(Final_Filename);
				Final_Filename.append(".o");
				Final_Filename.insert(0,"../build/");
				compile_Hook(current_file,Final_Filename,alone_Filename);
			}
        }
	}	
}