#ifndef ARTEFACT_H
#define ARTEFACT_H

#include "../database.h"
#include "../path.h"
#include <string>

namespace commands {
namespace artefact {
    // Artefact Type management
    void listArtefactTypes(Database& db, int projectId);
    void addArtefactType(Database& db, int projectId, const std::string& name, const std::string& description);
    void selectArtefactType(Database& db, Path& pth, int id);
    void removeArtefactType(Database& db, int id);

    // Artefact Field management
    void listArtefactFields(Database& db, int artefactTypeId);
    void addArtefactField(Database& db, int artefactTypeId, const std::string& fieldTitle,
                         int valueType, int maxLength, int maxValue, int minValue);
    void removeArtefactField(Database& db, int fieldId);

    // Artefact management
    void listArtefacts(Database& db, int artefactTypeId);
    void addArtefact(Database& db, int artefactTypeId, const std::string& title, int ownerId);
    void selectArtefact(Database& db, Path& pth, int id);
    void removeArtefact(Database& db, int id);

    // Artefact Data management
    void listArtefactData(Database& db, int artefactId);
    void setArtefactData(Database& db, int artefactId, int fieldId, const std::string& value);

    // Artefact to Activity linking
    void linkArtefactToActivity(Database& db, int artefactId, int activityId, double ratio);
    void listArtefactLinks(Database& db, int artefactId);
    void removeArtefactLink(Database& db, int linkId);

} // namespace artefact
} // namespace commands

#endif // ARTEFACT_H
