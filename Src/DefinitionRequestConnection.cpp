#include "DefinitionRequestConnection.hpp"
#include "Application.hpp"
#include "MainForm.hpp"

using namespace ArsLexis;

DefinitionRequestConnection::DefinitionRequestConnection(SocketConnectionManager& manager, const String& request):
    SimpleSocketConnection(manager, request),
    parser_(response())
{
}

DefinitionRequestConnection::~DefinitionRequestConnection()
{
}


void DefinitionRequestConnection::reportProgress()
{
    if (!(sending() || response().empty()))
        parser_.parseIncrement();
}

void DefinitionRequestConnection::finalize()
{
    parser_.parseIncrement(true);
    MainForm* form=static_cast<MainForm*>(Application::instance().getOpenForm(mainForm));
    if (form)
    {
        parser_.updateDefinition(form->definition());        form->update();
    }
    SimpleSocketConnection::finalize();
}