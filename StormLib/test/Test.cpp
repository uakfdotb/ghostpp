/*****************************************************************************/
/* StormLibTest.cpp                       Copyright (c) Ladislav Zezula 2003 */
/*---------------------------------------------------------------------------*/
/* This module uses very brutal test methods for StormLib. It extracts all   */
/* files from the archive with Storm.dll and with stormlib and compares them,*/
/* then tries to build a copy of the entire archive, then removes a few files*/
/* from the archive and adds them back, then compares the two archives, ...  */
/*---------------------------------------------------------------------------*/
/*   Date    Ver   Who  Comment                                              */
/* --------  ----  ---  -------                                              */
/* 25.03.03  1.00  Lad  The first version of StormLibTest.cpp                */
/*****************************************************************************/

#define _CRT_SECURE_NO_DEPRECATE
#include <io.h>
#include <conio.h>
#include <stdio.h>
#include <windows.h>
#include <mmsystem.h>

#define __STORMLIB_SELF__               // Don't use StormLib.lib
#define __LINK_STORM_DLL__
#include "StormLib.h"

#pragma warning(disable : 4505) 
#pragma comment(lib, "Winmm.lib")

//------------------------------------------------------------------------------
// Defines

#define MPQ_BLOCK_SIZE 0x1000

//-----------------------------------------------------------------------------
// Constants

static const char * szWorkDir = ".\\!Work";

static LPBYTE pbBuffer1 = NULL;
static LPBYTE pbBuffer2 = NULL;

int Compress_wave_stereo(char * pbOutBuffer, int * pdwOutLength, char * pbInBuffer, int dwInLength, int * pCmpType, int nCmpLevel);
int Decompress_wave_stereo(char * pbOutBuffer, int * pdwOutLength, char * pbInBuffer, int dwInLength);

//-----------------------------------------------------------------------------
// Local testing functions

static void clreol()
{
    printf("\r                                                                              \r");
}

static const char * GetPlainName(const char * szFileName)
{
    const char * szTemp;

    if((szTemp = strrchr(szFileName, '\\')) != NULL)
        szFileName = szTemp + 1;
    return szFileName;
}

int GetFirstDiffer(void * ptr1, void * ptr2, int nSize)
{
    char * buff1 = (char *)ptr1;
    char * buff2 = (char *)ptr2;
    int nDiffer;

    for(nDiffer = 0; nDiffer < nSize; nDiffer++)
    {
        if(*buff1++ != *buff2++)
            return nDiffer;
    }
    return -1;
}

static void ShowProcessedFile(const char * szFileName)
{
    char szLine[80];
    size_t nLength = strlen(szFileName);

    memset(szLine, 0x20, sizeof(szLine));
    szLine[sizeof(szLine)-1] = 0;

    if(nLength > sizeof(szLine)-1)
        nLength = sizeof(szLine)-1;
    memcpy(szLine, szFileName, nLength);
    printf("\r%s\r", szLine);
}


static void WINAPI CompactCB(void * /* lpParam */, DWORD dwWork, DWORD dwParam1, DWORD dwParam2)
{
    clreol();
    switch(dwWork)
    {
        case CCB_CHECKING_FILES:
            printf("Checking files in archive ...\r");
            break;

        case CCB_CHECKING_HASH_TABLE:
            printf("Checking hash table ...\r");
            break;

        case CCB_COPYING_NON_MPQ_DATA:
            printf("Copying non-MPQ data ...\r");
            break;

        case CCB_COMPACTING_FILES:
            printf("Compacting archive (%u of %u) ...\r", dwParam1, dwParam2);
            break;

        case CCB_CLOSING_ARCHIVE:
            printf("Closing archive ...\r");
            break;
    }
}

static int ExtractBytes(HANDLE hMpq, const char * szFileName, void * pBuffer, DWORD & dwBytes)
{
    HANDLE hFile = NULL;
    int nError = ERROR_SUCCESS;

    // Open the file
    if(nError == ERROR_SUCCESS)
    {
        if(!SFileOpenFileEx(hMpq, szFileName, 0, &hFile))
            nError = GetLastError();
    }

    // Read first block
    if(nError == ERROR_SUCCESS)
    {
        if(!SFileReadFile(hFile, pBuffer, dwBytes, &dwBytes, NULL))
            nError = GetLastError();
        if(nError == ERROR_HANDLE_EOF)
            nError = ERROR_SUCCESS;
    }

    // Close the file and cleanup
    if(hFile != NULL)
        SFileCloseFile(hFile);
    return nError;
}

static BOOL CompareArchivedFiles(const char * szFileName, HANDLE hFile1, HANDLE hFile2, DWORD dwBlockSize)
{
    DWORD dwRead1;                      // Number of bytes read (Storm.dll)
    DWORD dwRead2;                      // Number of bytes read (StormLib)
    BOOL bResult1 = FALSE;              // Result from Storm.dll
    BOOL bResult2 = FALSE;              // Result from StormLib
    int nDiff;
//  int nError = ERROR_SUCCESS;

    UNREFERENCED_PARAMETER(szFileName);

    for(;;)
    {
        // Read the file's content by both methods and compare the result
        memset(pbBuffer1, 0, dwBlockSize);
        memset(pbBuffer2, 0, dwBlockSize);
        bResult1 = SFileReadFile(hFile1, pbBuffer1, dwBlockSize, &dwRead1, NULL);
        bResult2 = SFileReadFile(hFile2, pbBuffer2, dwBlockSize, &dwRead2, NULL);
        if(bResult1 != bResult2)
        {
            printf("Different results from SFileReadFile, Mpq1 %u, Mpq2 %u\n", bResult1, bResult2);
            return FALSE;                         
        }

        // Test the number of bytes read
        if(dwRead1 != dwRead2)
        {
            printf("Different bytes read from SFileReadFile, Mpq1 %u, Mpq2 %u\n", dwRead1, dwRead2);
            return FALSE;
        }

        // No more bytes ==> OK
        if(dwRead1 == 0)
            return TRUE;
        
        // Test the content
        if((nDiff = GetFirstDiffer(pbBuffer1, pbBuffer2, dwRead1)) != -1)
        {
            return FALSE;
        }
    }
}

// Random read version
static int CompareArchivedFilesRR(const char * szFileName, HANDLE hFile1, HANDLE hFile2, DWORD dwBlockSize)
{
    DWORD dwFileSize1;                  // File size (Storm.dll)
    DWORD dwFileSize2;                  // File size (StormLib)
    DWORD dwRead1;                      // Number of bytes read (Storm.dll)
    DWORD dwRead2;                      // Number of bytes read (StormLib)
    BOOL bResult1 = FALSE;              // Result from Storm.dll
    BOOL bResult2 = FALSE;              // Result from StormLib
    int nError = ERROR_SUCCESS;

    // Test the file size
    dwFileSize1 = SFileGetFileSize(hFile1, NULL);
    dwFileSize2 = SFileGetFileSize(hFile2, NULL);
    if(dwFileSize1 != dwFileSize2)
    {
        printf("Different size from SFileGetFileSize, Storm.dll: %u, StormLib: %u !!!!\n", dwFileSize1, dwFileSize2);
        return ERROR_GEN_FAILURE;
    }

    for(int i = 0; i < 100; i++)
    {
        DWORD dwRandom   = rand() * rand();
        DWORD dwPosition = dwRandom % dwFileSize1;
        DWORD dwToRead   = dwRandom % dwBlockSize; 

        // Set the file pointer
        printf("RndRead: \"%s\", position %u, size %u ...\r", szFileName, dwPosition, dwToRead);
        dwRead1 = SFileSetFilePointer(hFile1, dwPosition, NULL, FILE_BEGIN);
        dwRead2 = SFileSetFilePointer(hFile2, dwPosition, NULL, FILE_BEGIN);
        if(dwRead1 != dwRead2)
        {
            printf("Difference returned by SFileSetFilePointer, Storm.dll: %u, StormLib: %u !!!!\n", dwRead1, dwRead2);
            nError = ERROR_READ_FAULT;
            break;
        }

        // Read the file's content by both methods and compare the result
        bResult1 = SFileReadFile(hFile1, pbBuffer1, dwToRead, &dwRead1, NULL);
        bResult2 = SFileReadFile(hFile2, pbBuffer2, dwToRead, &dwRead2, NULL);
        if(bResult1 != bResult2)
        {
            printf("Different results from SFileReadFile, Storm.dll: %u, StormLib: %u !!!!\n", bResult1, bResult2);
            nError = ERROR_READ_FAULT;
            break;
        }

        // Test the number of bytes read
        if(dwRead1 != dwRead2)
        {
            printf("Different bytes read from SFileReadFile, Storm.dll: %u, StormLib: %u !!!!\n", dwRead1, dwRead2);
            nError = ERROR_READ_FAULT;
            break;
        }
        
        // Test the content
        if(dwRead1 != 0 && memcmp(pbBuffer1, pbBuffer2, dwRead1))
        {
            printf("Different block content from SFileReadFile !!\n");
            nError = ERROR_READ_FAULT;
            break;
        }
    }
    clreol();
    return ERROR_SUCCESS;
}

static int TestBlockCompress(const char * szFileName)
{
    HANDLE hFile = INVALID_HANDLE_VALUE;
    LPBYTE pbDecompressed = NULL;
    LPBYTE pbCompressed = NULL;
    LPBYTE pbData = NULL;
    DWORD dwFileSize;
    int   nBlocks = 0;
    int nError = ERROR_SUCCESS;

    // Open the file
    if(nError == ERROR_SUCCESS)
    {
        hFile = CreateFile(szFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
        if(hFile == INVALID_HANDLE_VALUE)
            nError = GetLastError();
    }

    // Allocate buffers
    if(nError == ERROR_SUCCESS)
    {
        // Must allocate twice blocks due to probable bug in Storm.dll.
        // Storm.dll corrupts stack when uncompresses data with PKWARE DCL
        // and no compression occurs.
        pbDecompressed = new BYTE [MPQ_BLOCK_SIZE];
        pbCompressed = new BYTE [MPQ_BLOCK_SIZE];
        pbData = new BYTE[MPQ_BLOCK_SIZE];
        if(!pbDecompressed || !pbCompressed || !pbData)
            nError = ERROR_NOT_ENOUGH_MEMORY;
    }
    if(nError == ERROR_SUCCESS)
    {
        dwFileSize = GetFileSize(hFile, NULL);
        nBlocks    = dwFileSize / MPQ_BLOCK_SIZE;
        if(dwFileSize % MPQ_BLOCK_SIZE)
            nBlocks++;

        for(int i = 0x10; i < nBlocks; i++)
        {
            DWORD dwTransferred;
            int   nDcmpLength;
            int   nCmpLength;
            int   nCmpLevel = -1;
            int   nCmpType = 0;
            int   nCmp = MPQ_COMPRESSION_BZIP2;
            int   nDiff;

            clreol();
            printf("Testing compression of block %u\r", i + 1);

            // Load the block from the file
            SetFilePointer(hFile, i * MPQ_BLOCK_SIZE, NULL, FILE_BEGIN);
            ReadFile(hFile, pbData, MPQ_BLOCK_SIZE, &dwTransferred, NULL);
            if(dwTransferred == 0)
                continue;

            // Compress the block
            nCmpLength = dwTransferred;
            SCompCompress((char *)pbCompressed, &nCmpLength, (char *)pbData, dwTransferred, nCmp, nCmpType, nCmpLevel);

            // Uncompress the block
            nDcmpLength = dwTransferred;
            SCompDecompress((char *)pbDecompressed, &nDcmpLength, (char *)pbCompressed, nCmpLength);
  
            if(nDcmpLength != (int)dwTransferred)
            {
                printf("Number of uncompressed bytes does not agree with original data !!!\n");
                break;
            }
            if((nDiff = GetFirstDiffer(pbDecompressed, pbData, dwTransferred)) != -1)
            {
                printf("Decompressed block does not agree with the original data !!! (Offset 0x%08lX)\n", nDiff);
                break;
            }

            if(pbCompressed[MPQ_BLOCK_SIZE] != 0xFD)
            {
                printf("Damage after compressed block !!!\n");
                break;
            }
        }
    }

    // Cleanup
    if(pbData != NULL)
        delete [] pbData;
    if(pbCompressed != NULL)
        delete [] pbCompressed;
    if(pbDecompressed != NULL)
        delete [] pbDecompressed;
    if(hFile != INVALID_HANDLE_VALUE)
        CloseHandle(hFile);
    clreol();
    return nError;
}

static int TestArchiveOpenAndClose(const char * szMpqName, const char * szFileName)
{
    HANDLE hMpq = NULL;
    HANDLE hFile = NULL;
    DWORD dwMaxLocales = 0x100;
    LCID Locales[0x100];
    int nError = ERROR_SUCCESS;

    if(nError == ERROR_SUCCESS)
    {
        printf("Opening archive %s ...\n", szMpqName);
        if(!SFileCreateArchiveEx(szMpqName, OPEN_EXISTING, 0, &hMpq))
            nError = GetLastError();
    }

    if(nError == ERROR_SUCCESS)
    {
//      SFileEnumLocales(hMpq, szFileName, Locales, &dwMaxLocales, 0);
        SFileExtractFile(hMpq, szFileName, "E:\\Extracted.bin");
    }
/*
    if(nError == ERROR_SUCCESS && szFileName != NULL)
    {
        if(!SFileOpenFileEx(hMpq, szFileName, 0, &hFile))
        {
            nError = GetLastError();
            printf("%s - file integrity error\n", szFileName);
        }
    }

    if(nError == ERROR_SUCCESS)
    {
        LARGE_INTEGER FileSize;

        FileSize.LowPart = SFileGetFileSize(hFile, (LPDWORD)&FileSize.HighPart);
    }

    if(nError == ERROR_SUCCESS)
    {
        DWORD dwBytesRead;
        BYTE Buffer[0x1000];

        SFileReadFile(hFile, Buffer, sizeof(Buffer), &dwBytesRead, NULL);
    }

    if(hFile != NULL)
        SFileCloseFile(hFile);
*/
    if(hMpq != NULL)
        SFileCloseArchive(hMpq);
    return nError;
}

static int TestArchiveOpenAndCloseStorm(const char * szMpqName, const char * szFileName)
{
    HANDLE hMpq = NULL;
    HANDLE hFile = NULL;
    DWORD dwBytesRead = 0;
    BYTE Buffer[0x1000];
    int nError = ERROR_SUCCESS;

    if(nError == ERROR_SUCCESS)
    {
        printf("Opening archive %s ...\n", szMpqName);
        if(!StormOpenArchive(szMpqName, 0, 0, &hMpq))
            nError = GetLastError();
    }

    if(nError == ERROR_SUCCESS)
    {
        if(!StormOpenFileEx(hMpq, szFileName, 0, &hFile))
            nError = GetLastError();
    }

    if(nError == ERROR_SUCCESS)
    {
        StormReadFile(hFile, Buffer, sizeof(Buffer), &dwBytesRead, NULL);
    }

    if(hFile != NULL)
        StormCloseFile(hFile);
    if(hMpq != NULL)
        StormCloseArchive(hMpq);
    return nError;
}

static int TestFindFiles(const char * szMpqName)
{
    TMPQFile * hf;
    HANDLE hFile;
    HANDLE hMpq = NULL;
    BYTE Buffer[100];
    int nError = ERROR_SUCCESS;
    int nFiles = 0;
    int nFound = 0;

    // Open the archive
    if(nError == ERROR_SUCCESS)
    {
        printf("Opening \"%s\" for finding files ...\n", szMpqName);
        if(!SFileCreateArchiveEx(szMpqName, OPEN_EXISTING, 0, &hMpq))
            nError = GetLastError();
    }

    // Compact the archive
    if(nError == ERROR_SUCCESS)
    {
        SFILE_FIND_DATA sf;
        HANDLE hFind;
        DWORD dwExtraDataSize;
        BOOL bFound = TRUE;

        hFind = SFileFindFirstFile(hMpq, "*", &sf, "c:\\Tools32\\ListFiles\\ListFile.txt");
        while(hFind != NULL && bFound != FALSE)
        {
            if(SFileOpenFileEx(hMpq, sf.cFileName, 0, &hFile))
            {
                hf = (TMPQFile *)hFile;
                SFileReadFile(hFile, Buffer, sizeof(Buffer));
                nFiles++;

                if(sf.dwFileFlags & MPQ_FILE_HAS_EXTRA)
                {
                    dwExtraDataSize = hf->pdwBlockPos[hf->nBlocks + 1] - hf->pdwBlockPos[hf->nBlocks];
                    if(dwExtraDataSize != 0)
                        nFound++;
                }

                SFileCloseFile(hFile);
            }

            bFound = SFileFindNextFile(hFind, &sf);
        }
    }

    if(hMpq != NULL)
        SFileCloseArchive(hMpq);
    if(nError == ERROR_SUCCESS)
        printf("Search complete\n");
    return nError;
}

static int TestMpqCompacting(const char * szMpqName)
{
    HANDLE hMpq = NULL;
    int nError = ERROR_SUCCESS;

    // Open the archive
    if(nError == ERROR_SUCCESS)
    {
        printf("Opening \"%s\" for compacting ...\n", szMpqName);
        if(!SFileCreateArchiveEx(szMpqName, OPEN_EXISTING, 0, &hMpq))
            nError = GetLastError();
    }
/*
    if(nError == ERROR_SUCCESS)
    {
        char * szFileName = "Campaign\\Human\\Human01.pud";

        printf("Deleting file %s ...\r", szFileName);
        if(!SFileRemoveFile(hMpq, szFileName))
            nError = GetLastError();
    }
*/
    // Compact the archive
    if(nError == ERROR_SUCCESS)
    {
        printf("Compacting archive ...\r");
        SFileSetCompactCallback(hMpq, CompactCB, NULL);
        if(!SFileCompactArchive(hMpq, NULL))
            nError = GetLastError();
    }

    if(hMpq != NULL)
        SFileCloseArchive(hMpq);
    if(nError == ERROR_SUCCESS)
        printf("Compacting complete (No errors)\n");
    return nError;
}


static int TestCreateArchiveV2(const char * szMpqCopyName, const char * szFileName)
{
    HANDLE hMpq = NULL;                 // Handle of created archive 
    HANDLE hFile = INVALID_HANDLE_VALUE;
    char szAddFileName[MAX_PATH] = "";
    int nError = ERROR_SUCCESS;

    // Create the new file
    hFile = CreateFile(szMpqCopyName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
    if(hFile == INVALID_HANDLE_VALUE)
        nError = GetLastError();

    // Write some data
    if(nError == ERROR_SUCCESS)
    {
        SetFilePointer(hFile, 0x100000, NULL, FILE_BEGIN);
        if(!SetEndOfFile(hFile))
            nError = GetLastError();
        CloseHandle(hFile);
    }

    // Well, now create the MPQ archive
    if(nError == ERROR_SUCCESS)
    {
        printf("Creating %s ...\n", szMpqCopyName);
        SFileSetLocale(LANG_CZECH);
        if(!SFileCreateArchiveEx(szMpqCopyName,
                                 MPQ_CREATE_ARCHIVE_V2 | OPEN_ALWAYS,
                                 0x100, // dwHashTableSize
                                &hMpq))
            nError = GetLastError();
        SFileSetLocale(LANG_NEUTRAL);
    }

    // Add the same file multiple times to make archive bigger than 4 GB
    if(nError == ERROR_SUCCESS)
    {
        for(int i = 1; i < 4; i++)
        {
            sprintf(szAddFileName, "Data%04u.iso", i);
            printf("Adding %s ...\n", szAddFileName);
            if(!SFileAddFile(hMpq, szFileName, szAddFileName, MPQ_FILE_ENCRYPTED))
            {
                printf("Cannot add the file (%u-th pass)\n", i);
                break;
            }
        }
    }

    if(hMpq != NULL)
        SFileCloseArchive(hMpq);
    return nError;
}


static int TestCreateArchiveCopy(const char * szMpqName, const char * szMpqCopyName, const char * szListFile)
{
    char   szLocalFile[MAX_PATH] = "";
    HANDLE hMpq1 = NULL;                // Handle of existing archive
    HANDLE hMpq2 = NULL;                // Handle of created archive 
    HANDLE hFile = INVALID_HANDLE_VALUE;
    DWORD dwHashTableSize = 0;
    int nError = ERROR_SUCCESS;

    // If no listfile or an empty one, use NULL
    if(szListFile == NULL || *szListFile == 0)
        szListFile = NULL;

    // Create the new file
    hFile = CreateFile(szMpqCopyName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
    if(hFile == INVALID_HANDLE_VALUE)
        nError = GetLastError();

    // Write some data
    if(nError == ERROR_SUCCESS)
    {
        SetFilePointer(hFile, 0x100000, NULL, FILE_BEGIN);
        if(!SetEndOfFile(hFile))
            nError = GetLastError();
        CloseHandle(hFile);
    }

    // Open the existing MPQ archive
    if(nError == ERROR_SUCCESS)
    {
        printf("Opening %s ...\n", szMpqName);
        if(!SFileOpenArchive(szMpqName, 0, 0, &hMpq1))
            nError = GetLastError();
    }

    // Well, now create the MPQ archive
    if(nError == ERROR_SUCCESS)
    {
        printf("Creating %s ...\n", szMpqCopyName);
        dwHashTableSize = (DWORD)SFileGetFileInfo(hMpq1, SFILE_INFO_HASH_TABLE_SIZE);
        if(!SFileCreateArchiveEx(szMpqCopyName, OPEN_EXISTING, dwHashTableSize, &hMpq2))
            nError = GetLastError();
    }

    // Copy all files from one archive to another
    if(nError == ERROR_SUCCESS)
    {
        SFILE_FIND_DATA wf;
        HANDLE hFind = SFileFindFirstFile(hMpq1, "*", &wf, szListFile);
        BOOL bResult = TRUE;

        printf("Adding files\n");

        while(hFind != NULL && bResult == TRUE)
        {
            ShowProcessedFile(wf.cFileName);
            SFileSetLocale(wf.lcLocale);

            // Create the local file name
            sprintf(szLocalFile, "%s\\%s", szWorkDir, GetPlainName(wf.cFileName));
            if(!SFileExtractFile(hMpq1, wf.cFileName, szLocalFile))
            {
                nError = GetLastError();
                printf("Failed to extract %s\n", wf.cFileName);
                break;
            }

            wf.dwFileFlags &= ~MPQ_FILE_HAS_EXTRA;
            wf.dwFileFlags &= ~MPQ_FILE_EXISTS;

            if(!SFileAddFile(hMpq2, szLocalFile, wf.cFileName, wf.dwFileFlags))
            {
                nError = GetLastError();
                printf("Failed to add the file %s into archive\n", wf.cFileName);
                break;
            }

            // Delete the added file
            DeleteFile(szLocalFile);

            // Find the next file
            bResult = SFileFindNextFile(hFind, &wf);
        }

        // Delete the extracted file in the case of an error
        if(nError != ERROR_SUCCESS)
            DeleteFile(szLocalFile);

        // Close the search handle
        if(hFind != NULL)
            SFileFindClose(hFind);
    }

    // Close both archives
    if(hMpq2 != NULL)
        SFileCloseArchive(hMpq2);
    if(hMpq1 != NULL)
        SFileCloseArchive(hMpq1);
    if(nError == ERROR_SUCCESS)
        printf("MPQ creating complete (No errors)\n");
    return nError;
}

static int TestCompareTwoArchives(
    const char * szMpqName1,
    const char * szMpqName2,
    const char * szListFile,
    DWORD dwBlockSize)
{
    TMPQArchive * ha1 = NULL;
    TMPQArchive * ha2 = NULL;
    HANDLE hMpq1 = NULL;                // Handle of the first archive
    HANDLE hMpq2 = NULL;                // Handle of the second archive
    HANDLE hFile1 = NULL;
    HANDLE hFile2 = NULL;
    int nError = ERROR_SUCCESS;

    // If no listfile or an empty one, use NULL
    if(szListFile == NULL || *szListFile == 0)
        szListFile = NULL;

    // Allocate both buffers
    pbBuffer1 = (LPBYTE)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwBlockSize);
    pbBuffer2 = (LPBYTE)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwBlockSize);
    if(pbBuffer1 == NULL || pbBuffer2 == NULL)
        nError = ERROR_NOT_ENOUGH_MEMORY;

    printf("=============== Comparing MPQ archives ===============\n");

    // Open the first MPQ archive
    if(nError == ERROR_SUCCESS)
    {
        printf("Opening %s ...\n", szMpqName1);
        if(!SFileOpenArchive(szMpqName1, 0, 0, &hMpq1))
            nError = GetLastError();
        ha1 = (TMPQArchive *)hMpq1;
    }

    // Open the second MPQ archive
    if(nError == ERROR_SUCCESS)
    {
        printf("Opening %s ...\n", szMpqName2);
        if(!SFileOpenArchive(szMpqName2, 0, 0, &hMpq2))
            nError = GetLastError();
        ha2 = (TMPQArchive *)hMpq2;
    }

    // Compare the header
    if(nError == ERROR_SUCCESS)
    {
        if(ha1->pHeader->dwHeaderSize != ha2->pHeader->dwHeaderSize)
            printf(" - Header size is different\n");
        if(ha1->pHeader->wFormatVersion != ha2->pHeader->wFormatVersion)
            printf(" - Format version is different\n");
        if(ha1->pHeader->wBlockSize != ha2->pHeader->wBlockSize)
            printf(" - Block size Format version is different\n");
        if(ha1->pHeader->dwHashTableSize != ha2->pHeader->dwHashTableSize)
            printf(" - Hash table size is different\n");
        if(ha1->pHeader->dwBlockTableSize != ha2->pHeader->dwBlockTableSize)
            printf(" - Block table size is different\n");
    }

    // Find all files in the first archive and compare them
    if(nError == ERROR_SUCCESS)
    {
        SFILE_FIND_DATA wf;
        TMPQFile * hf1;
        TMPQFile * hf2;
        HANDLE hFind = SFileFindFirstFile(hMpq1, "*", &wf, szListFile);
        BOOL bResult = TRUE;

        while(hFind != NULL && bResult == TRUE)
        {
            ShowProcessedFile(wf.cFileName);
            SFileSetLocale(wf.lcLocale);

            // Open the first file
            if(!SFileOpenFileEx(hMpq1, wf.cFileName, 0, &hFile1))
            {
                printf("Failed to open the file %s in the first archive\n", wf.cFileName);
                continue;
            }

            if(!SFileOpenFileEx(hMpq2, wf.cFileName, 0, &hFile2))
            {
                printf("Failed to open the file %s in the second archive\n", wf.cFileName);
                continue;
            }

            hf1 = (TMPQFile *)hFile1;
            hf2 = (TMPQFile *)hFile2;

            // Compare the file sizes
            if(hf1->pBlock->dwFSize != hf2->pBlock->dwFSize)
                printf(" - %s different size (%u x %u)\n", wf.cFileName, hf1->pBlock->dwFSize, hf2->pBlock->dwFSize);

            if(hf1->pBlock->dwFlags != hf2->pBlock->dwFlags)
                printf(" - %s different flags (%08lX x %08lX)\n", wf.cFileName, hf1->pBlock->dwFlags, hf2->pBlock->dwFlags);

            if(!CompareArchivedFiles(wf.cFileName, hFile1, hFile2, 0x1001))
                printf(" - %s different content\n", wf.cFileName);

            // Close both files
            SFileCloseFile(hFile2);
            SFileCloseFile(hFile1);
            hFile2 = hFile1 = NULL;

            // Find the next file
            bResult = SFileFindNextFile(hFind, &wf);
        }

        // Close all handles
        if(hFile2 != NULL)
            SFileCloseFile(hFile2);
        if(hFile1 != NULL)
            SFileCloseFile(hFile1);
        if(hFind != NULL)
            SFileFindClose(hFind);
    }

    // Close both archives
    clreol();
    printf("================ MPQ compare complete ================\n");
    if(hMpq2 != NULL)
        SFileCloseArchive(hMpq2);
    if(hMpq1 != NULL)
        SFileCloseArchive(hMpq1);
    if(pbBuffer2 != NULL)
        HeapFree(GetProcessHeap(), 0, pbBuffer2);
    if(pbBuffer1 != NULL)
        HeapFree(GetProcessHeap(), 0, pbBuffer1);
    return nError;
}


//-----------------------------------------------------------------------------
// Main
// 
// The program must be run with two command line arguments
//
// Arg1 - The source MPQ name (for testing reading and file find)
// Arg2 - Listfile name
//

void main(int argc, char ** argv)
{
    char szMpqName[MAX_PATH] = "";
    char szMpqCopyName[MAX_PATH] = "";
    char szListFile[MAX_PATH] = "";
    DWORD dwHashTableSize = 0x8000;     // Size of the hash table.
    int nError = ERROR_SUCCESS;

    UNREFERENCED_PARAMETER(dwHashTableSize);

    // Mix the random number generator
    srand(GetTickCount());

    // Use command line arguments, if any
    if(argc >= 2)
        strcpy(szMpqName, argv[1]);
    if(argc >= 3)
        strcpy(szListFile, argv[2]);

    // When not given or not exist, read them
/*
    while(szMpqName[0] == 0 || _access(szMpqName, 0) != 0)
    {
        printf("No MPQ archive given\n");
        printf("Press any key to exit ...\n");
        _getch();
        return;
    }
*/
    // Set the lowest priority to allow running in the background
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);

    // Create the name of the MPQ copy
    if(nError == ERROR_SUCCESS)
    {
        char * szTemp = NULL;

        strcpy(szMpqCopyName, szMpqName);
        if((szTemp = strrchr(szMpqCopyName, '.')) != NULL)
            strcpy(szTemp, "_copy.mpq");
    }

    // Create the working directory
    if(nError == ERROR_SUCCESS)
    {
        if(!CreateDirectory(szWorkDir, NULL))
            nError = GetLastError();
        if(nError == ERROR_ALREADY_EXISTS)
            nError = ERROR_SUCCESS;
    }

    // Test compression methods
//  if(nError == ERROR_SUCCESS)
//      nError = TestBlockCompress("E:\\TestFile.txt");

    // Test the archive open and close
    if(nError == ERROR_SUCCESS)
        nError = TestArchiveOpenAndClose(szMpqName, (char *)1);

    // Test the archive open and close
//  if(nError == ERROR_SUCCESS)
//      nError = TestArchiveOpenAndCloseStorm(szMpqName, "war3map.w3d");

//  if(nError == ERROR_SUCCESS)
//      nError = TestFindFiles(szMpqName);

    // Compact the archive        
//  if(nError == ERROR_SUCCESS)
//      nError = TestMpqCompacting(szMpqName);

    // Create a big MPQ archive
//  if(nError == ERROR_SUCCESS)
//      nError = TestCreateArchiveV2(szMpqCopyName, "E:\\false.exe");

    // Create copy of the archive, appending some bytes before the MPQ header
//  if(nError == ERROR_SUCCESS)
//      nError = TestCreateArchiveCopy(szMpqName, szMpqCopyName, szListFile);

//  if(nError == ERROR_SUCCESS)
//      nError = TestCompareTwoArchives(argv[1], argv[2], NULL, 0x1001);

    // Remove the working directory
    RemoveDirectory(szWorkDir);
    clreol();
    if(nError != ERROR_SUCCESS)
        printf("One or more errors occurred when testing StormLib\n");
    printf("Work complete. Press any key to exit ...\n");
    _getch();
}
