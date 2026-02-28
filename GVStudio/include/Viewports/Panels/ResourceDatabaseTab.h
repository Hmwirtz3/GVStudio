#pragma once 

#include "Database/ResourceDatabase.h"

class ResourceDatabaseTab
{
public:

	void DrawResourceFolderTree(const ResourceDatabase& resourceDatabase);
	void Setvisible(bool visible);
	bool IsVisible();


private:

	
	void DrawResourceNode(const ResourceNode* node);
	


private:

	bool m_isVisible = true;
};