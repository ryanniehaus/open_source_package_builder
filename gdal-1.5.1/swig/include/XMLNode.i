/******************************************************************************
 * $Id: XMLNode.i 10967 2007-03-15 20:04:37Z warmerdam $
 *
 * Project:  GDAL SWIG Interface
 * Purpose:  GDAL XML SWIG Interface declarations.
 * Author:   Tamas Szekeres (szekerest@gmeil.com)
 *
 ******************************************************************************
 * Copyright (c) 2005, Tamas Szekeres
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *****************************************************************************/

%rename (XMLNodeType) CPLXMLNodeType;
typedef enum 
{
    CXT_Element = 0,
    CXT_Text = 1,
    CXT_Attribute = 2,  
    CXT_Comment = 3,    
    CXT_Literal = 4     
} CPLXMLNodeType;

%rename (XMLNode) _CPLXMLNode;
%rename (Type) _CPLXMLNode::eType;
%rename (Value) _CPLXMLNode::pszValue;
%rename (Next) _CPLXMLNode::psNext;
%rename (Child) _CPLXMLNode::psChild;
typedef struct _CPLXMLNode
{
%immutable;
    CPLXMLNodeType      eType;       
    char                *pszValue;   
    struct _CPLXMLNode *psNext;     
    struct _CPLXMLNode *psChild;
%mutable;  
} CPLXMLNode;

%extend CPLXMLNode 
{
    CPLXMLNode(const char *pszString) 
    {
        return CPLParseXMLString( pszString );     
    }

    ~CPLXMLNode() 
    {
        CPLDestroyXMLNode( self );
    }
    
    char *SerializeXMLTree( )
    {
        return CPLSerializeXMLTree( self );
    }
    
    CPLXMLNode *SearchXMLNode( const char *pszElement )
    {
        return CPLSearchXMLNode(self, pszElement);
    }
    
    CPLXMLNode *GetXMLNode( const char *pszPath )
    {
        return CPLGetXMLNode( self, pszPath );
    }
    
    const char *GetXMLValue( const char *pszPath, 
                            const char *pszDefault )
    {
        return CPLGetXMLValue( self, pszPath, pszDefault );                    
    }
    
    void AddXMLChild( CPLXMLNode *psChild )
    {
        return CPLAddXMLChild( self, psChild );
    }
    
    int RemoveXMLChild( CPLXMLNode *psChild )
    {
        return CPLRemoveXMLChild( self, psChild );
    }
    
    void AddXMLSibling( CPLXMLNode *psNewSibling )
    {
        return CPLAddXMLSibling( self, psNewSibling );
    }
    
    CPLXMLNode *CreateXMLElementAndValue( const char *pszName, 
                                         const char *pszValue )
    {
        return CPLCreateXMLElementAndValue( self, pszName, pszValue );                                 
    }
    
    %newobject CloneXMLTree;
    CPLXMLNode *CloneXMLTree( CPLXMLNode *psTree )
    {
        return CPLCloneXMLTree( psTree );
    }
    
    int SetXMLValue( const char *pszPath,
                    const char *pszValue )
    {
        return CPLSetXMLValue( self,  pszPath, pszValue );           
    }
    
    void StripXMLNamespace( const char *pszNamespace, 
                           int bRecurse )
    {
        CPLStripXMLNamespace( self, pszNamespace, bRecurse );                  
    }
}
