/*
Copyright (C) 2012 Sebastian Herbord. All rights reserved.

This file is part of Mod Organizer.

Mod Organizer is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Mod Organizer is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Mod Organizer.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef MODINFO_H
#define MODINFO_H

#include "imodinterface.h"
#include "versioninfo.h"

class PluginContainer;
class QDir;
class QDateTime;

#include <QMutex>
#include <QSharedPointer>
#include <QString>
#include <QStringList>
#include <QColor>

#include <boost/function.hpp>

#include <map>
#include <set>
#include <vector>

namespace MOBase { class IPluginGame; }
namespace MOShared { class DirectoryEntry; }

/**
 * @brief Represents meta information about a single mod.
 *
 * Represents meta information about a single mod. The class interface is used
 * to manage the mod collection
 *
 **/
class ModInfo : public QObject, public MOBase::IModInterface
{

  Q_OBJECT

public:

  typedef QSharedPointer<ModInfo> Ptr;

  static QString s_HiddenExt;

  enum EConflictFlag {
    FLAG_CONFLICT_OVERWRITE,
    FLAG_CONFLICT_OVERWRITTEN,
    FLAG_CONFLICT_MIXED,
    FLAG_CONFLICT_REDUNDANT,
    FLAG_ARCHIVE_LOOSE_CONFLICT_OVERWRITE,
    FLAG_ARCHIVE_LOOSE_CONFLICT_OVERWRITTEN,
    FLAG_ARCHIVE_CONFLICT_OVERWRITE,
    FLAG_ARCHIVE_CONFLICT_OVERWRITTEN,
    FLAG_ARCHIVE_CONFLICT_MIXED,
    FLAG_OVERWRITE_CONFLICT,
  };

  enum EFlag {
    FLAG_INVALID,
    FLAG_BACKUP,
    FLAG_SEPARATOR,
    FLAG_OVERWRITE,
    FLAG_FOREIGN,
    FLAG_HIDDEN_FILES,
    FLAG_NOTENDORSED,
    FLAG_NOTES,
    FLAG_PLUGIN_SELECTED,
    FLAG_ALTERNATE_GAME,
    FLAG_TRACKED,
  };

  enum EHighlight {
    HIGHLIGHT_NONE = 0,
    HIGHLIGHT_INVALID = 1,
    HIGHLIGHT_CENTER = 2,
    HIGHLIGHT_IMPORTANT = 4,
    HIGHLIGHT_PLUGIN = 8
  };

  enum EEndorsedState {
    ENDORSED_FALSE,
    ENDORSED_TRUE,
    ENDORSED_UNKNOWN,
    ENDORSED_NEVER,
    ENDORSED_CANNOT_ENDORSE,
  };

  enum ETrackedState {
    TRACKED_FALSE,
    TRACKED_TRUE,
    TRACKED_UNKNOWN,
  };

  enum EModType {
    MOD_DEFAULT,
    MOD_DLC,
    MOD_CC
  };


public:

  /**
   * @brief read the mod directory and Mod ModInfo objects for all subdirectories
   **/
  static void updateFromDisc(const QString &modDirectory,
                             MOShared::DirectoryEntry **directoryStructure,
                             PluginContainer *pluginContainer,
                             bool displayForeign,
                             std::size_t refreshThreadCount,
                             MOBase::IPluginGame const *game);

  static void clear() { s_Collection.clear(); s_ModsByName.clear(); s_ModsByModID.clear(); }

  /**
   * @brief retrieve the number of mods
   *
   * @return number of mods
   **/
  static unsigned int getNumMods();

  /**
   * @brief retrieve a ModInfo object based on its index
   *
   * @param index the index to look up. the maximum is getNumMods() - 1
   * @return a reference counting pointer to the mod info.
   * @note since the pointer is reference counting, the pointer remains valid even if the collection is refreshed in a different thread
   **/
  static ModInfo::Ptr getByIndex(unsigned int index);

  /**
   * @brief retrieve a ModInfo object based on its nexus mod id
   *
   * @param modID the nexus mod id to look up
   * @return a reference counting pointer to the mod info
   * @todo in its current form, this function is broken! There may be multiple mods with the same nexus id,
   *       this function will return only one of them
   **/
  static std::vector<ModInfo::Ptr> getByModID(QString game, int modID);

  /**
   * @brief retrieve a ModInfo object based on its name
   *
   * @param name the name to look up
   * @return a reference counting pointer to the mod info
   * @note since the pointer is reference counter, the pointer remains valid even if the collection is refreshed in a different thread
   **/
  static ModInfo::Ptr getByName(const QString &name);

  /**
   * @brief remove a mod by index
   *
   * this physically deletes the specified mod from the disc and updates the ModInfo collection
   * but not other structures that reference mods
   * @param index index of the mod to delete
   * @return true if removal was successful, fals otherwise
   **/
  static bool removeMod(unsigned int index);

  /**
   * @brief retrieve the mod index by the mod name
   *
   * @param name name of the mod to look up
   * @return the index of the mod. If the mod doesn't exist, UINT_MAX is returned
   **/
  static unsigned int getIndex(const QString &name);

  /**
   * @brief find the first mod that fulfills the filter function (after no particular order)
   * @param filter a function to filter by. should return true for a match
   * @return index of the matching mod or UINT_MAX if there wasn't a match
   */
  static unsigned int findMod(const boost::function<bool (ModInfo::Ptr)> &filter);

  /**
   * @brief run a limited batch of mod update checks for "newest version" information
   */
  static void manualUpdateCheck(PluginContainer *pluginContainer, QObject *receiver, std::multimap<QString, int> IDs);

  /**
   * @brief query nexus information for every mod and update the "newest version" information
   * @return true if any mods are checked for update
   **/
  static bool checkAllForUpdate(PluginContainer *pluginContainer, QObject *receiver);

  static std::set<QSharedPointer<ModInfo>> filteredMods(QString gameName, QVariantList updateData, bool addOldMods = false, bool markUpdated = false);

  /**
   * @brief create a new mod from the specified directory and add it to the collection
   * @param dir directory to create from
   * @return pointer to the info-structure of the newly created/added mod
   */
  static ModInfo::Ptr createFrom(PluginContainer *pluginContainer, const MOBase::IPluginGame *game, const QDir &dir, MOShared::DirectoryEntry **directoryStructure);

  /**
   * @brief create a new "foreign-managed" mod from a tuple of plugin and archives
   * @param espName name of the plugin
   * @param bsaNames names of archives
   * @return a new mod
   */
  static ModInfo::Ptr createFromPlugin(const QString &modName, const QString &espName, const QStringList &bsaNames, ModInfo::EModType modType, 
    const MOBase::IPluginGame* game, MOShared::DirectoryEntry **directoryStructure, PluginContainer *pluginContainer);

  // whether the given name is used for separators
  //
  static bool isSeparatorName(const QString& name);

  // whether the given name is used for backups
  //
  static bool isBackupName(const QString& name);

  // whether the given name is used for regular mods
  //
  static bool isRegularName(const QString& name);

  virtual bool isRegular() const { return false; }

  virtual bool isEmpty() const { return false; }

  /**
   * @brief test if there is a newer version of the mod
   *
   * test if there is a newer version of the mod. This does NOT cause
   * information to be retrieved from the nexus, it will only test version information already
   * available locally. Use checkAllForUpdate() to update this version information
   *
   * @return true if there is a newer version
   **/
  virtual bool updateAvailable() const = 0;

  /**
   * @return true if the update currently available is ignored
   */
  virtual bool updateIgnored() const = 0;

  /**
   * @brief test if the "newest" version of the mod is older than the installed version
   *
   * test if there is a newer version of the mod. This does NOT cause
   * information to be retrieved from the nexus, it will only test version information already
   * available locally. Use checkAllForUpdate() to update this version information
   *
   * @return true if the newest version is older than the installed one
   **/
  virtual bool downgradeAvailable() const = 0;

  /**
   * @brief request an update of nexus description for this mod.
   *
   * This requests mod information from the nexus. This is an asynchronous request,
   * so there is no immediate effect of this call.
   * Right now, Mod Organizer interprets the "newest version" and "description" from the
   * response, though the description is only stored in memory
   *
   **/
  virtual bool updateNXMInfo() = 0;

  /**
   * @brief assign or unassign the specified category
   *
   * Every mod can have an arbitrary number of categories assigned to it
   *
   * @param categoryID id of the category to set
   * @param active determines wheter the category is assigned or unassigned
   * @note this function does not test whether categoryID actually identifies a valid category
   **/
  virtual void setCategory(int categoryID, bool active) = 0;

  /**
   * @brief changes the comments (manually set information displayed in the mod list) for this mod
   * @param comments new comments
   */
  virtual void setComments(const QString &comments) = 0;

  /**
   * @brief change the notes (manually set information) for this mod
   * @param notes new notes
   */
  virtual void setNotes(const QString &notes) = 0;

  /**
   * @brief set/change the source game of this mod
   *
   * @param gameName the source game shortName
   */
  virtual void setGameName(const QString& gameName) = 0;

  /**
   * @brief set the name of this mod
   *
   * set the name of this mod. This will also update the name of the
   * directory that contains this mod
   *
   * @param name new name of the mod
   * @return true on success, false if the new name can't be used (i.e. because the new
   *         directory name wouldn't be valid)
   **/
  virtual bool setName(const QString& name) = 0;

  /**
   * @brief set/change the nexus mod id of this mod
   *
   * @param modID the nexus mod id
   **/
  virtual void setNexusID(int modID) = 0;

  /**
   * @brief set/change the version of this mod
   * @param version new version of the mod
   */
  virtual void setVersion(const MOBase::VersionInfo &version) override;

  /**
  * @brief Controls if mod should be highlighted based on plugin selection
  * @param isSelected whether or not the plugin has a selected mod
  **/
  virtual void setPluginSelected(const bool &isSelected);

  /**
   * @brief set the newest version of this mod on the nexus
   *
   * this can be used to overwrite the version of a mod without actually
   * updating the mod
   *
   * @param version the new version to use
   * @todo this function should be made obsolete. All queries for mod information should go through
   *       this class so no public function for this change is required
   **/
  virtual void setNewestVersion(const MOBase::VersionInfo &version) = 0;

  /**
   * @brief sets the repository that was used to download the mod
   */
  virtual void setRepository(const QString &) {}

  /**
   * @brief changes/updates the nexus description text
   * @param description the current description text
   */
  virtual void setNexusDescription(const QString &description) = 0;

  /**
   * @brief sets the file this mod was installed from
   * @param fileName name of the file
   */
  virtual void setInstallationFile(const QString &fileName) = 0;

  /**
   * @brief sets the category id from a nexus category id. Conversion to MO id happens internally
   * @param categoryID the nexus category id
   * @note if a mapping is not possible, the category is set to the default value
   */
  virtual void addNexusCategory(int categoryID) = 0;

  virtual void addCategory(const QString &categoryName) override;
  virtual bool removeCategory(const QString &categoryName) override;
  virtual QStringList categories() const override;

  /**
   * update the endorsement state for the mod. This only changes the
   * buffered state, it does not sync with Nexus
   * @param endorsed the new endorsement state
   */
  virtual void setIsEndorsed(bool endorsed) = 0;

  /**
   * set the mod to "i don't intend to endorse". The mod will not show as unendorsed but can still be endorsed
   */
  virtual void setNeverEndorse() = 0;

  /**
   * set the mod to "cannot be rated".  Mod authors can designate their mod as not able to be rated on nexus.
  */
  virtual void setCannotEndorse() = 0;

  /**
   * update the tracked state for the mod.  This only changes the
   * buffered state, it does not sync with Nexus
   * @param tracked the new tracked state
   */
  virtual void setIsTracked(bool tracked) = 0;

  /**
   * @brief delete the mod from the disc. This does not update the global ModInfo structure or indices
   * @return true if the mod was successfully removed
   **/
  virtual bool remove() = 0;

  /**
   * @brief endorse or un-endorse the mod. This will sync with nexus!
   * @param doEndorse if true, the mod is endorsed, if false, it's un-endorsed.
   * @note if doEndorse doesn't differ from the current value, nothing happens.
   */
  virtual void endorse(bool doEndorse) = 0;

  /**
   * @brief track or untrack the mod.  This will sync with nexus!
   * @param doTrack if true, the mod is tracked, if false, it's untracked.
   * @note if doTrack doesn't differ from the current value, nothing happens.
   */
  virtual void track(bool doTrack) = 0;

  /**
   * @brief clear all caches held for this mod
   */
  virtual void clearCaches() {}

  /**
   * @brief getter for the mod name
   *
   * @return the mod name
   **/
  virtual QString name() const = 0;

  /**
   * @brief getter for an internal name. This is usually the same as the regular name, but with special mod types it might be
   *        this is used to distinguish between mods that have the same visible name
   * @return internal mod name
   */
  virtual QString internalName() const { return name(); }

  /**
   * @brief getter for the mod path
   *
   * @return the (absolute) path to the mod
   **/
  virtual QString absolutePath() const = 0;

  /**
   * @brief getter for the installation file
   *
   * @return file used to install this mod from
   */
  virtual QString getInstallationFile() const = 0;

  /**
   * @return version object for machine based comparisons
   **/
  virtual MOBase::VersionInfo getVersion() const { return m_Version; }

  /**
   * @brief getter for the newest version number of this mod
   *
   * @return newest version of the mod
   **/
  virtual MOBase::VersionInfo getNewestVersion() const = 0;

  /**
   * @return the repository from which the file was downloaded. Only relevant regular mods
   */
  virtual QString repository() const { return ""; }

  /**
   * @brief ignore the newest version for updates
   */
  virtual void ignoreUpdate(bool ignore) = 0;

  /**
   * @brief getter for the nexus mod id
   *
   * @return the nexus mod id. may be 0 if the mod id isn't known or doesn't exist
   **/
  virtual int getNexusID() const = 0;

  /**
  * @brief getter for the source game repository
  *
  * @return the source game repository. should default to the active game.
  **/
  virtual QString getGameName() const = 0;

  /**
   * @return the fixed priority of mods of this type or INT_MIN if the priority of mods
   *         needs to be user-modifiable. Can be < 0 to force a priority below user-modifable mods
   *         or INT_MAX to force priority above all user-modifiables
   */
  virtual int getFixedPriority() const = 0;

  /**
   * @return true if the mod is always enabled
   */
  virtual bool alwaysEnabled() const { return false; }

  /**
   * @return true if the mod can be updated
   */
  virtual bool canBeUpdated() const { return false; }

  /**
   * @return the mod update check expiration date
   */
  virtual QDateTime getExpires() const = 0;

  /**
   * @return true if the mod can be enabled/disabled
   */
  virtual bool canBeEnabled() const { return false; }

  /**
   * @return a list of flags for this mod
   */
  virtual std::vector<EFlag> getFlags() const = 0;

  /**
   * @return a list of conflict flags for this mod
   */
  virtual std::vector<EConflictFlag> getConflictFlags() const = 0;

  /**
   * @return a list of content types contained in a mod
   */
  virtual const std::set<int>& getContents() const = 0;

  /**
   * @brief test if the specified flag is set for this mod
   * @param flag the flag to test
   * @return true if the flag is set, false otherwise
   */
  bool hasFlag(EFlag flag) const;

  /**
   * @brief test if any of the provided flags are set for this mod
   * @param flags the flags to test
   * @return true if any of the flags are set, false otherwise
   */
  bool hasAnyOfTheseFlags(std::vector<ModInfo::EFlag> flags) const;

  /**
   * @brief Test if the mod contains the specified content.
   *
   * @param content ID of the content to test.
   *
   * @return true if the content is there, false otherwise.
   */
  virtual bool hasContent(int content) const = 0;

  /**
   * @return an indicator if and how this mod should be highlighted by the UI
   */
  virtual int getHighlight() const { return HIGHLIGHT_NONE; }

  /**
   * @return list of names of ini tweaks
   **/
  virtual std::vector<QString> getIniTweaks() const = 0;

  /**
   * @return a description about the mod, to be displayed in the ui
   */
  virtual QString getDescription() const = 0;

  /**
   * @return the nexus file status (aka category ID)
   */
  virtual int getNexusFileStatus() const = 0;


  /**
   * @brief sets the file status (category ID) from Nexus
   * @param status the status id of the installed file
   */
  virtual void setNexusFileStatus(int status) = 0;

  /**
   * @return comments for this mod
   */
  virtual QString comments() const = 0;

  /**
   * @return notes for this mod
   */
  virtual QString notes() const = 0;

  /**
   * @return creation time of this mod
   */
  virtual QDateTime creationTime() const = 0;

  /**
   * @return nexus description of the mod (html)
   */
  virtual QString getNexusDescription() const = 0;

  /**
   * @brief get the last time nexus was checked for file updates on this mod
   */
  virtual QDateTime getLastNexusUpdate() const = 0;

  /**
   * @brief set the last time nexus was checked for file updates on this mod
   */
  virtual void setLastNexusUpdate(QDateTime time) = 0;

  /**
   * @return last time nexus was queried for infos on this mod
   */
  virtual QDateTime getLastNexusQuery() const = 0;

  /**
   * @brief set the last time nexus was queried for info on this mod
   */
  virtual void setLastNexusQuery(QDateTime time) = 0;

  /**
   * @return last time the mod was updated on Nexus
   */
  virtual QDateTime getNexusLastModified() const = 0;

  /**
   * @brief set the last time the mod was updated on Nexus
   */
  virtual void setNexusLastModified(QDateTime time) = 0;

  /**
   * @return a list of files that, if they exist in the data directory are treated as files in THIS mod
   */
  virtual QStringList stealFiles() const { return QStringList(); }

  /**
   * @return a list of archives belonging to this mod (as absolute file paths)
   */
  virtual QStringList archives(bool checkOnDisk = false) = 0;

  /*
   *@return the color choosen by the user for the mod/separator
   */
  virtual QColor getColor() const { return QColor(); }

  /*
   *@return true if the color has been set successfully.
   */
  virtual void setColor(QColor) { }

  /**
   * @brief adds the information that a file has been installed into this mod
   * @param modId id of the mod installed
   * @param fileId id of the file installed
   */
  virtual void addInstalledFile(int modId, int fileId) = 0;

  /**
   * @brief test if the mod belongs to the specified category
   *
   * @param categoryID the category to test for.
   * @return true if the mod belongs to the specified category
   * @note this does not verify the id actually identifies a category
   **/
  bool categorySet(int categoryID) const;

  /**
   * @brief retrive the whole list of categories (as ids) this mod belongs to
   *
   * @return list of categories
   **/
  const std::set<int> &getCategories() const { return m_Categories; }

  /**
   * @return id of the primary category of this mod
   */
  int getPrimaryCategory() const { return m_PrimaryCategory; }

  /**
   * @brief sets the new primary category of the mod
   * @param categoryID the category to set
   */
  virtual void setPrimaryCategory(int categoryID) { m_PrimaryCategory = categoryID; }

  /**
   * @return true if this mod is considered "valid", that is: it contains data used by the game
   **/
  virtual bool isValid() const = 0;

  /**
   * @return true if the file has been endorsed on nexus
   */
  virtual EEndorsedState endorsedState() const { return ENDORSED_NEVER; }

  /**
   * @return true if the file is being tracked on nexus
   */
  virtual ETrackedState trackedState() const { return TRACKED_FALSE; }

  /**
   * @brief updates the mod to flag it as converted in order to ignore the alternate game warning
   */
  virtual void markConverted(bool) {}

  /**
  * @brief updates the mod to flag it as valid in order to ignore the invalid game data flag
  */
  virtual void markValidated(bool) {}

  /**
   * @brief reads meta information from disk
   */
  virtual void readMeta() {}

  /**
   * @brief stores meta information back to disk
   */
  virtual void saveMeta() {}

  /**
   * @return retrieve list of mods (as mod index) that are overwritten by this one. Updates may be delayed
   */
  virtual std::set<unsigned int> getModOverwrite() const { return std::set<unsigned int>(); }

  /**
   * @return list of mods (as mod index) that overwrite this one. Updates may be delayed
   */
  virtual std::set<unsigned int> getModOverwritten() const { return std::set<unsigned int>(); }

  /**
   * @return retrieve list of mods (as mod index) with archives that are overwritten by this one. Updates may be delayed
  */
  virtual std::set<unsigned int> getModArchiveOverwrite() const { return std::set<unsigned int>(); }

  /**
  * @return list of mods (as mod index) with archives that overwrite this one. Updates may be delayed
  */
  virtual std::set<unsigned int> getModArchiveOverwritten() const { return std::set<unsigned int>(); }

  /**
  * @return retrieve list of mods (as mod index) with archives that are overwritten by thos mod's loose files. Updates may be delayed
  */
  virtual std::set<unsigned int> getModArchiveLooseOverwrite() const { return std::set<unsigned int>(); }

  /**
  * @return list of mods (as mod index) with loose files that overwrite this one's archive files. Updates may be delayed
  */
  virtual std::set<unsigned int> getModArchiveLooseOverwritten() const { return std::set<unsigned int>(); }

  /**
   * @brief update conflict information
   */
  virtual void doConflictCheck() const {}

  /**
   * @brief sets whether this mod uses a custom url
   **/
  virtual void setHasCustomURL(bool) {}

  /**
   * @brief returns whether this mod uses a custom url
   **/
  virtual bool hasCustomURL() const { return false; }

  /**
   * @brief sets the custom url
   **/
  virtual void setCustomURL(QString const &) {}

  /**
   * @brief returns the custom url
   **/
  virtual QString getCustomURL() const { return ""; }

  /**
   * If hasCustomURL() is true and getCustomURL() is not empty, tries to parse
   * the url using QUrl::fromUserInput() and returns it. Otherwise, returns an
   * empty QUrl.
   **/
  QUrl parseCustomURL() const;

public slots:

  /**
   * @brief Notify this mod that the content of the disk may have changed.
   */
  virtual void diskContentModified() = 0;

signals:

  /**
   * @brief emitted whenever the information of a mod changes
   *
   * @param success true if the mod details were updated successfully, false if not
   **/
  void modDetailsUpdated(bool success);

protected:

  /**
   *
   */
  ModInfo(PluginContainer *pluginContainer);

  /**
   * @brief Prefetch content for this mod.
   *
   * This method can be used to prefetch content from the mod, e.g., for isValid()
   * or getContents(). This method will only be called when first creating the mod
   * using multiple threads for all the mods.
   */
  virtual void prefetch() = 0;

  static void updateIndices();
  static bool ByName(const ModInfo::Ptr &LHS, const ModInfo::Ptr &RHS);

private:

  static void createFromOverwrite(PluginContainer *pluginContainer,
                                  const MOBase::IPluginGame* game,
                                  MOShared::DirectoryEntry **directoryStructure);

protected:

  static std::vector<ModInfo::Ptr> s_Collection;
  static std::map<QString, unsigned int> s_ModsByName;

  int m_PrimaryCategory;
  std::set<int> m_Categories;

  MOBase::VersionInfo m_Version;

  bool m_PluginSelected = false;

private:

  static QMutex s_Mutex;
  static std::map<std::pair<QString, int>, std::vector<unsigned int> > s_ModsByModID;
  static int s_NextID;

};


#endif // MODINFO_H
