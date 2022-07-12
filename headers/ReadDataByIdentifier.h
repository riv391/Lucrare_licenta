#ifndef READDATABYIDENTIFIER_H_
#define READDATABYIDENTIFIER_H_
#include "header.h"

typedef struct {
	char* name;
	DataIdentifier* identifier;
} RDBIIdentifiers;

extern RDBIIdentifiers standardIds[];

extern ConfigPanel RDBIPanel;

extern ServiceParam _RDBI;

int _RequestPanelRequest();

int _ResultPanelUpdate();

void ChangeIdentifier(GtkWidget*, void*);

int CreateRDBIPanel();

#endif
