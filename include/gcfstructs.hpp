#pragma once
#include <cstdint>
#include <span>
#include <numeric>

#pragma pack(push,1)
namespace gcf{

    enum class manifest_flags       {
        user_config_file = 0x00000001,
        launch_file = 0x00000002,
        locked = 0x00000008,
        nocache_file = 0x00000020,
        versioned_uc_file = 0x00000040, // uc = user config?
        purge_file = 0x00000080,
        encrypted_file = 0x00000100,
        read_only_file = 0x00000200,
        hidden_file = 0x00000400,
        executable_file = 0x00000800,
        file = 0x00004000,
    };

    enum class descriptor_version : std::int32_t {
        current_version = 1
    };

    enum class cache_type : std::int32_t {
        none = 0,
        one_file_fixed_block = 1,
        manifest_only = 2
    };

    enum class cache_state : std::int32_t {
        clean = 0,
        dirty = 1,
    };

    struct cache_descriptor{
        descriptor_version version_desc; // always 1
        cache_type type; // always 1
        std::uint32_t cache_version; // up to 6
        std::uint32_t appid; // always 1
        std::uint32_t app_version_id;
        cache_state state;
        std::uint32_t write_flag; // should be always 0
        std::uint32_t file_size;
        std::uint32_t block_size; //always 0x2000
        std::uint32_t max_entries; // aka blockcount
        std::uint32_t checksum;
        inline cache_descriptor* compute_checksum() {
            std::span<uint8_t> self{reinterpret_cast<uint8_t*>(this), sizeof(*this)-4};
            uint32_t chksum = std::accumulate(self.begin(),self.end(),0);
            this->checksum = chksum;
            return this;
        }
        inline cache_descriptor* ptr() { return this;}
    };


    struct bat_block {
        enum class size_t : std::int32_t {
            e16bit = 0x0,
            e32bit = 0x1
        };
        std::uint32_t max_entries;
        std::uint32_t next_free_entry;
        bat_block::size_t size;
        std::uint32_t checksum;
        inline bat_block* calculate_checksum() { 
            this->checksum = max_entries + next_free_entry + static_cast<std::int32_t>(size);
            return this;
        };
    };

    struct file_fixed_diectory_header{
        std::uint32_t max_entries;
        std::uint32_t entries_in_use;
        std::uint32_t next_free_block;
        std::uint32_t literally_nothing[4]; // no, seriously literally nothing | memset(&u32FixedPart[3], 0, 16);
        std::uint32_t checksum;
        inline file_fixed_diectory_header* compute_checksum(){ this->checksum = max_entries + entries_in_use + next_free_block; return this;};
        inline file_fixed_diectory_header* ptr() { return this; }
    };

    // WRONG
    enum class block_flags : std::uint16_t {
        decrypted_probably = 0, // also loose (decrypted?)
        //unused = 0xbfff, // according to steamcooker
        used_block = 0x8000,
        extracted_file = 0x4000,
        loose = 1,
        compressed_and_crypted = 2,
        crypted = 4
    };

    inline constexpr block_flags operator|(block_flags a, block_flags b) {
        return static_cast<block_flags>(
            static_cast<std::uint16_t>(a) | static_cast<std::uint16_t>(b)
        );
    }

    inline constexpr block_flags operator&(block_flags a, block_flags b) {
        return static_cast<block_flags>(
            static_cast<std::uint16_t>(a) & static_cast<std::uint16_t>(b)
        );
    }

    inline block_flags& operator|=(block_flags& a, block_flags b) {
        a = a | b;
        return a;
    }

    inline block_flags& operator&=(block_flags& a, block_flags b) {
        a = a & b;
        return a;
    }

    inline constexpr block_flags operator~(block_flags a) {
        return static_cast<block_flags>(
            ~static_cast<std::uint16_t>(a)
        );
    }

    enum class compression_type : uint32_t {
        uncompressed = 0,
        compressed,
        compressed_and_encrypted,
        encrypted
    };
    constexpr static inline uint32_t stupid_lookup_table[] = {1,2,0,3};

    struct file_fixed_directory_entry {
        //block_flags flags;
        block_flags flags;
        std::uint16_t open;

        std::uint32_t offset;
        std::uint32_t len;
        std::uint32_t bat_entry;
        std::uint32_t next_entry;
        std::uint32_t prev_entry;
        std::uint32_t manifest_node; // or (also known as) directory index
        inline file_fixed_directory_entry* ptr(){ return this; }

        inline compression_type block_compression_type() {
            auto bruh = (static_cast<std::uint32_t>(flags) & 7) - 1;
            if (bruh <= 3){
                return static_cast<compression_type>(stupid_lookup_table[bruh]);
            }
            return static_cast<compression_type>(0);
        };

        inline void set_compression_type(compression_type type){
            this->flags = this->flags & static_cast<block_flags>(0xFFF8);
            switch (type) {
                using enum compression_type;
                case compressed:
                    this->flags = this->flags | static_cast<block_flags>(1);
                    break;
                case compressed_and_encrypted:
                    this->flags = this->flags | static_cast<block_flags>(2);
                    break;
                case encrypted:
                    this->flags = this->flags | static_cast<block_flags>(4);
                    break;
            }
        }
        inline bool is_encrypted(){
            auto bruh = static_cast<uint32_t>(block_compression_type());
            if (bruh - 2 <= 1){
                return true;
            }
            return false;
        }   

    };


    constexpr inline std::uint32_t file_fixed_fs_tree_version = 1;

    struct file_fixed_fs_tree_header {
        std::uint32_t current_version = file_fixed_fs_tree_version;
        std::uint32_t trickle_keys = 0; // always 0
        inline file_fixed_fs_tree_header* ptr() { return this; }
    };


    inline constexpr std::uint32_t checksum_format_version = 1;

    struct file_fixed_checksum_header{
        std::uint32_t format_version = checksum_format_version;
        std::uint32_t size_of_checksum_table;
        inline file_fixed_checksum_header* ptr() { return this; }
    };

    // checksum table struct skipped from this file
    // checksum table struct skipped from this file
    // checksum table struct skipped from this file
    // checksum table struct skipped from this file
    // checksum table struct skipped from this file
    // checksum table struct skipped from this file
    // checksum table struct skipped from this file
    // checksum table struct skipped from this file

    /*
        if ( fwrite(SerializedVersion, m_u32SizeOfChecksumTable, 1u, pFile) != 1
            || fwrite(&this->m_uAppVersionId, 4u, 1u, pFile) != 1 ) // <--- real steam just memdumps malloced struct to the file here, we have it already dumped in another file
        {
            exception = (common::CErrorCodeException *)__cxa_allocate_exception((size_t)&cszDesc.m_iterIndex);
            Grid::ICache::CWriteException::CWriteException(v10, v12);
            goto LABEL_4;
        }
    */

    struct file_fixed_checksum_footer {
        std::uint32_t app_version;
        inline file_fixed_checksum_footer* ptr() { return this; }
    };

    struct data_block {
        std::uint32_t max_entries;
        std::uint32_t block_size;
        std::uint32_t data_start;
        std::uint32_t entries_in_use;
        std::uint32_t checksum;
        inline data_block* calculate_checksum(){
            this->checksum = max_entries + block_size + data_start + entries_in_use;
            return this;
        }
    };
}
#pragma pack(pop)