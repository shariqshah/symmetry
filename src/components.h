#ifndef components_H
#define components_H

enum Component
{
	C_TRANSFORM = 0,
	C_MODEL,
	C_CAMERA,
	C_LIGHT,
	C_RIGIDBODY,
	MAX_COMPONENTS
};

inline static const char* comp_to_str(enum Component component)
{
	const char* str = 0;
	switch(component)
	{
	case C_TRANSFORM :    str = "TRANSFORM";      break;
	case C_MODEL :        str = "MODEL";          break;
	case C_CAMERA :       str = "CAMERA";         break;
	case C_LIGHT :        str = "LIGHT";          break;
	case C_RIGIDBODY :    str = "RIGIDBODY";      break;
	case MAX_COMPONENTS : str = "MAX_COMPONENTS"; break;
	}
	return str;
}

#endif
