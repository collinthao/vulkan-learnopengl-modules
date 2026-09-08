#include <iostream>
#include <fstream>
/*
inline std::ostream& operator << (std::ostream& os, InstanceData& i)
{
	os << i.pos;
	os << i.scale;
	os << "Rotation: " << i.rot << '\n';
	os << "Id: " << i.id << '\n';
	return os;
}
*/

inline std::ostream& operator << (std::ostream& os, const glm::vec2& v)
{
	os << "x: " << v.x << " | y: " << v.y << '\n';
	return os;
}

inline std::ostream& operator << (std::ostream& os, const glm::vec3& v)
{
	os << "x: " << v.x << " | y: " << v.y << " | z: " << v.z << '\n';
	return os;
}

inline std::ostream& operator << (std::ostream& os, const glm::vec4& v)
{
	os << "x: " << v.x << " | y: " << v.y << " | z: " << v.z <<  " | w: " << v.w << '\n';
	return os;
}

inline std::ostream& operator << (std::ostream& os, const glm::mat4& m)
{
	for (int i = 0; i < 4; i++)
	{
		std::cout << m[i] << '\n';
	};	
	return os;
}
/*
template <typename T, size_t N>
inline std::ostream& operator << (std::ostream& os, const std::array<T,N>& a)
{
	os << '{' << a[0] << ',' << a[1] << '}';
	return os;
}*/

inline void printMemory(const char* label)
{
    std::ifstream file("/proc/self/status");
    std::string line;

    while (std::getline(file, line))
    {
        if (line.starts_with("VmRSS:"))
        {
            std::cout << label << ": " << line << '\n';
            break;
        }
    }
}
