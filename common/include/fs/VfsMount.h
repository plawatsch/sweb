// Projectname: SWEB
// Simple operating system for educational purposes

#ifndef VfsMount_h___
#define VfsMount_h___

#include "types.h"

class Superblock;
class Dentry;

// Mount flags
// Only MS_RDONLY is supported by now.

/// Mount the Filesystem read-only
#define MS_RDONLY 1

//---------------------------------------------------------------------------
/**
 * VfsMount
 *
 * Definitions for mount interface. This describes the in the kernel
 * build linkedlist with mounted filesystems.
 */
class VfsMount
{
 protected:

  /// Points to the parent filesystem on which this filesystem is mounted on.
  VfsMount *mnt_parent_;

  /// Points to the Dentry of the mount directory of this filesystem.
  Dentry *mnt_mountpoint_;

  /// Points to the Dentry of the root directory of this filesystem.
  Dentry *mnt_root_;

  /// Points to the superblock object of this filesystem.
  Superblock *mnt_sb_;

  /// The mnt_flags_ field of the descriptor stores the value of several flags
  /// that specify how some kinds of files in the mounted filesystem are
  /// handled.
  int32 mnt_flags_;

  ///// Head of the parent list of descriptors (relative to this filesystem).
  // List *mnt_mounts_;

  ///// Pointers for the parent list of descriptors (relative to the parent
  ///// filesystem).
  // List *mnt_child_;

 public:

  /// default-constructor
  VfsMount();

  /// constructor
  VfsMount(VfsMount* parent, Dentry * mountpoint, Dentry* root,
      Superblock* superblock, int32 flags);

  /// destructor
  virtual ~VfsMount();

  // void put_mnt(VfsMount *mnt);

  // void remove_mnt(VfsMount *mnt);

  // VfsMount* get_mnt();

  /// get the parent-VfsMount of the VfsMount
  VfsMount *getParent() const;

  /// get the mount-point of the VfsMount
  Dentry *getMountPoint() const;

  /// get the ROOT-directory of the VfsMount
  Dentry *getRoot() const;

  /// get the superblock fo the VfsMount
  Superblock *getSuperblock() const;

  /// get the flags
  int32 getFlags() const;

  //NOTE: only used as workaround
  void clear();
  
};

#endif // Vfsmount_h___


