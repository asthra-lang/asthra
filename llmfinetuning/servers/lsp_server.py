#!/usr/bin/env python3
"""
Asthra Language Server Protocol (LSP) Server
Provides AI-powered tooling support for Asthra programming language
Integrates with fine-tuned Asthra models for intelligent code completion
"""

import asyncio
import json
import logging
import os
import sys
import time
from pathlib import Path
from typing import Dict, List, Optional, Any, Union
from urllib.parse import urlparse, unquote

# LSP Protocol implementation
try:
    from pygls import uris
    from pygls.lsp.methods import (
        COMPLETION, HOVER, SIGNATURE_HELP, DEFINITION, REFERENCES,
        DOCUMENT_SYMBOL, WORKSPACE_SYMBOL, CODE_ACTION, DIAGNOSTIC,
        TEXT_DOCUMENT_DID_OPEN, TEXT_DOCUMENT_DID_CHANGE, TEXT_DOCUMENT_DID_SAVE
    )
    from pygls.lsp.types import (
        CompletionItem, CompletionItemKind, CompletionList, CompletionParams,
        Hover, MarkupContent, MarkupKind, Position, Range,
        SignatureHelp, SignatureInformation, ParameterInformation,
        Location, DocumentSymbol, SymbolKind, WorkspaceSymbol,
        CodeAction, CodeActionKind, CodeActionParams,
        Diagnostic, DiagnosticSeverity,
        TextDocumentItem, TextDocumentContentChangeEvent,
        DidOpenTextDocumentParams, DidChangeTextDocumentParams, DidSaveTextDocumentParams
    )
    from pygls.server import LanguageServer
    HAS_PYGLS = True
except ImportError:
    print("❌ pygls not installed. Install with: pip install pygls")
    HAS_PYGLS = False

# OpenAI client for model integration
try:
    from openai import OpenAI
    HAS_OPENAI = True
except ImportError:
    print("❌ OpenAI client not available. Install with: pip install openai")
    HAS_OPENAI = False

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)

class AsthraiLanguageServer:
    """Language Server for Asthra with AI-powered features."""
    
    def __init__(self, model_api_base: str = "http://localhost:8000/v1"):
        if not HAS_PYGLS:
            raise RuntimeError("pygls is required for LSP server")
        
        self.server = LanguageServer('asthra-lsp', 'v1.0.0')
        self.model_api_base = model_api_base
        self.client = None
        self.documents = {}  # Cache of open documents
        self.asthra_symbols = {}  # Cache of Asthra symbols per document
        
        # Initialize OpenAI client for model communication
        if HAS_OPENAI:
            self.client = OpenAI(
                base_url=model_api_base,
                api_key="not-required"
            )
        
        self._register_handlers()
    
    def _register_handlers(self):
        """Register LSP method handlers."""
        
        @self.server.feature(TEXT_DOCUMENT_DID_OPEN)
        async def did_open(ls, params: DidOpenTextDocumentParams):
            """Handle document open event."""
            doc = params.text_document
            self.documents[doc.uri] = {
                'content': doc.text,
                'version': doc.version,
                'language_id': doc.language_id
            }
            logger.info(f"Opened document: {doc.uri}")
            
            # Trigger initial analysis
            await self._analyze_document(doc.uri, doc.text)
        
        @self.server.feature(TEXT_DOCUMENT_DID_CHANGE)
        async def did_change(ls, params: DidChangeTextDocumentParams):
            """Handle document change event."""
            doc = params.text_document
            changes = params.content_changes
            
            # Update document content
            if doc.uri in self.documents:
                for change in changes:
                    if hasattr(change, 'range') and change.range:
                        # Incremental change
                        self._apply_incremental_change(doc.uri, change)
                    else:
                        # Full document change
                        self.documents[doc.uri]['content'] = change.text
                
                self.documents[doc.uri]['version'] = doc.version
                
                # Trigger analysis with debouncing
                await self._debounced_analysis(doc.uri)
        
        @self.server.feature(TEXT_DOCUMENT_DID_SAVE)
        async def did_save(ls, params: DidSaveTextDocumentParams):
            """Handle document save event."""
            doc = params.text_document
            logger.info(f"Saved document: {doc.uri}")
            
            # Trigger full analysis on save
            if doc.uri in self.documents:
                content = self.documents[doc.uri]['content']
                await self._analyze_document(doc.uri, content)
        
        @self.server.feature(COMPLETION)
        async def completion(ls, params: CompletionParams):
            """Provide code completion."""
            doc_uri = params.text_document.uri
            position = params.position
            
            if doc_uri not in self.documents:
                return CompletionList(is_incomplete=False, items=[])
            
            content = self.documents[doc_uri]['content']
            completion_items = await self._get_completions(content, position, params.context)
            
            return CompletionList(
                is_incomplete=False,
                items=completion_items
            )
        
        @self.server.feature(HOVER)
        async def hover(ls, params):
            """Provide hover information."""
            doc_uri = params.text_document.uri
            position = params.position
            
            if doc_uri not in self.documents:
                return None
            
            content = self.documents[doc_uri]['content']
            hover_info = await self._get_hover_info(content, position)
            
            if hover_info:
                return Hover(
                    contents=MarkupContent(
                        kind=MarkupKind.Markdown,
                        value=hover_info
                    )
                )
            return None
        
        @self.server.feature(SIGNATURE_HELP)
        async def signature_help(ls, params):
            """Provide signature help."""
            doc_uri = params.text_document.uri
            position = params.position
            
            if doc_uri not in self.documents:
                return None
            
            content = self.documents[doc_uri]['content']
            signatures = await self._get_signature_help(content, position)
            
            if signatures:
                return SignatureHelp(signatures=signatures)
            return None
        
        @self.server.feature(DOCUMENT_SYMBOL)
        async def document_symbols(ls, params):
            """Provide document symbols."""
            doc_uri = params.text_document.uri
            
            if doc_uri not in self.documents:
                return []
            
            content = self.documents[doc_uri]['content']
            symbols = await self._extract_symbols(content)
            
            return symbols
        
        @self.server.feature(CODE_ACTION)
        async def code_actions(ls, params: CodeActionParams):
            """Provide code actions."""
            doc_uri = params.text_document.uri
            range_param = params.range
            context = params.context
            
            if doc_uri not in self.documents:
                return []
            
            content = self.documents[doc_uri]['content']
            actions = await self._get_code_actions(content, range_param, context)
            
            return actions
    
    async def _analyze_document(self, doc_uri: str, content: str):
        """Analyze document and publish diagnostics."""
        try:
            diagnostics = await self._get_diagnostics(content)
            self.server.publish_diagnostics(doc_uri, diagnostics)
            
            # Extract and cache symbols
            symbols = await self._extract_symbols(content)
            self.asthra_symbols[doc_uri] = symbols
            
        except Exception as e:
            logger.error(f"Error analyzing document {doc_uri}: {e}")
    
    async def _debounced_analysis(self, doc_uri: str):
        """Debounced document analysis to avoid excessive API calls."""
        # Wait a bit for more changes
        await asyncio.sleep(0.5)
        
        if doc_uri in self.documents:
            content = self.documents[doc_uri]['content']
            await self._analyze_document(doc_uri, content)
    
    async def _get_completions(self, content: str, position: Position, context) -> List[CompletionItem]:
        """Get AI-powered code completions."""
        if not self.client:
            return self._get_static_completions()
        
        try:
            # Extract context around cursor
            lines = content.split('\n')
            current_line = lines[position.line] if position.line < len(lines) else ""
            prefix = current_line[:position.character]
            
            # Build context for the model
            context_lines = max(0, position.line - 10)
            context_content = '\n'.join(lines[context_lines:position.line + 1])
            
            # Generate completion using the fine-tuned model
            prompt = f"""Complete this Asthra code:

{context_content}

Provide only the completion for the current line, starting from where the cursor is positioned."""
            
            response = self.client.chat.completions.create(
                model="asthra-gemma3-4b",
                messages=[
                    {"role": "system", "content": "You are an expert Asthra programmer providing code completions."},
                    {"role": "user", "content": prompt}
                ],
                max_tokens=256,
                temperature=0.3,
                stop=["\n\n", "```"]
            )
            
            completion_text = response.choices[0].message.content.strip()
            
            # Create completion item
            completion_items = [
                CompletionItem(
                    label=completion_text[:50] + "..." if len(completion_text) > 50 else completion_text,
                    kind=CompletionItemKind.Text,
                    detail="AI-generated completion",
                    documentation="Generated by fine-tuned Asthra model",
                    insert_text=completion_text
                )
            ]
            
            # Add static completions as fallback
            completion_items.extend(self._get_static_completions())
            
            return completion_items
            
        except Exception as e:
            logger.error(f"Error getting AI completions: {e}")
            return self._get_static_completions()
    
    def _get_static_completions(self) -> List[CompletionItem]:
        """Get static Asthra language completions."""
        static_items = [
            # Keywords
            CompletionItem(label="pub", kind=CompletionItemKind.Keyword),
            CompletionItem(label="priv", kind=CompletionItemKind.Keyword),
            CompletionItem(label="fn", kind=CompletionItemKind.Keyword),
            CompletionItem(label="struct", kind=CompletionItemKind.Keyword),
            CompletionItem(label="enum", kind=CompletionItemKind.Keyword),
            CompletionItem(label="impl", kind=CompletionItemKind.Keyword),
            CompletionItem(label="let", kind=CompletionItemKind.Keyword),
            CompletionItem(label="mut", kind=CompletionItemKind.Keyword),
            CompletionItem(label="if", kind=CompletionItemKind.Keyword),
            CompletionItem(label="else", kind=CompletionItemKind.Keyword),
            CompletionItem(label="for", kind=CompletionItemKind.Keyword),
            CompletionItem(label="while", kind=CompletionItemKind.Keyword),
            CompletionItem(label="match", kind=CompletionItemKind.Keyword),
            CompletionItem(label="return", kind=CompletionItemKind.Keyword),
            
            # Types
            CompletionItem(label="i32", kind=CompletionItemKind.TypeParameter),
            CompletionItem(label="i64", kind=CompletionItemKind.TypeParameter),
            CompletionItem(label="f32", kind=CompletionItemKind.TypeParameter),
            CompletionItem(label="f64", kind=CompletionItemKind.TypeParameter),
            CompletionItem(label="bool", kind=CompletionItemKind.TypeParameter),
            CompletionItem(label="char", kind=CompletionItemKind.TypeParameter),
            CompletionItem(label="string", kind=CompletionItemKind.TypeParameter),
            
            # Common patterns
            CompletionItem(
                label="function template",
                kind=CompletionItemKind.Snippet,
                insert_text="pub fn ${1:function_name}(${2:params}) -> ${3:return_type} {\n    ${4:// TODO: implement}\n}",
                documentation="Function template"
            ),
            CompletionItem(
                label="struct template",
                kind=CompletionItemKind.Snippet,
                insert_text="pub struct ${1:StructName} {\n    ${2:field}: ${3:Type},\n}",
                documentation="Struct template"
            ),
        ]
        
        return static_items
    
    async def _get_hover_info(self, content: str, position: Position) -> Optional[str]:
        """Get hover information using AI."""
        if not self.client:
            return None
        
        try:
            lines = content.split('\n')
            if position.line >= len(lines):
                return None
            
            current_line = lines[position.line]
            
            # Extract word at position
            line_chars = list(current_line)
            start = position.character
            end = position.character
            
            # Find word boundaries
            while start > 0 and (line_chars[start - 1].isalnum() or line_chars[start - 1] == '_'):
                start -= 1
            while end < len(line_chars) and (line_chars[end].isalnum() or line_chars[end] == '_'):
                end += 1
            
            if start == end:
                return None
            
            word = current_line[start:end]
            if not word:
                return None
            
            # Get context
            context_start = max(0, position.line - 5)
            context_end = min(len(lines), position.line + 6)
            context_lines = lines[context_start:context_end]
            context_content = '\n'.join(context_lines)
            
            # Ask AI for explanation
            prompt = f"""Explain this Asthra code element: `{word}`

Context:
```asthra
{context_content}
```

Provide a brief, helpful explanation of what this element does or represents."""
            
            response = self.client.chat.completions.create(
                model="asthra-gemma3-4b",
                messages=[
                    {"role": "system", "content": "You are an expert Asthra programmer providing code explanations."},
                    {"role": "user", "content": prompt}
                ],
                max_tokens=200,
                temperature=0.3
            )
            
            return response.choices[0].message.content.strip()
            
        except Exception as e:
            logger.error(f"Error getting hover info: {e}")
            return None
    
    async def _get_signature_help(self, content: str, position: Position) -> List[SignatureInformation]:
        """Get function signature help."""
        # For now, return static signature help
        # This could be enhanced with AI-powered analysis
        return []
    
    async def _extract_symbols(self, content: str) -> List[DocumentSymbol]:
        """Extract document symbols."""
        symbols = []
        lines = content.split('\n')
        
        for i, line in enumerate(lines):
            line = line.strip()
            
            # Function symbols
            if line.startswith(('pub fn', 'priv fn', 'fn')):
                fn_name = self._extract_function_name(line)
                if fn_name:
                    symbol = DocumentSymbol(
                        name=fn_name,
                        kind=SymbolKind.Function,
                        range=Range(
                            start=Position(line=i, character=0),
                            end=Position(line=i, character=len(line))
                        ),
                        selection_range=Range(
                            start=Position(line=i, character=line.find(fn_name)),
                            end=Position(line=i, character=line.find(fn_name) + len(fn_name))
                        )
                    )
                    symbols.append(symbol)
            
            # Struct symbols
            elif line.startswith(('pub struct', 'priv struct', 'struct')):
                struct_name = self._extract_struct_name(line)
                if struct_name:
                    symbol = DocumentSymbol(
                        name=struct_name,
                        kind=SymbolKind.Struct,
                        range=Range(
                            start=Position(line=i, character=0),
                            end=Position(line=i, character=len(line))
                        ),
                        selection_range=Range(
                            start=Position(line=i, character=line.find(struct_name)),
                            end=Position(line=i, character=line.find(struct_name) + len(struct_name))
                        )
                    )
                    symbols.append(symbol)
        
        return symbols
    
    def _extract_function_name(self, line: str) -> Optional[str]:
        """Extract function name from function declaration."""
        import re
        match = re.search(r'fn\s+(\w+)', line)
        return match.group(1) if match else None
    
    def _extract_struct_name(self, line: str) -> Optional[str]:
        """Extract struct name from struct declaration."""
        import re
        match = re.search(r'struct\s+(\w+)', line)
        return match.group(1) if match else None
    
    async def _get_diagnostics(self, content: str) -> List[Diagnostic]:
        """Get document diagnostics using AI."""
        if not self.client:
            return []
        
        try:
            # Use AI to analyze code for issues
            prompt = f"""Analyze this Asthra code for potential issues, errors, or warnings:

```asthra
{content}
```

Report any syntax errors, type errors, or style issues. Format as JSON with line numbers and descriptions."""
            
            response = self.client.chat.completions.create(
                model="asthra-gemma3-4b",
                messages=[
                    {"role": "system", "content": "You are an expert Asthra compiler providing code analysis."},
                    {"role": "user", "content": prompt}
                ],
                max_tokens=500,
                temperature=0.1
            )
            
            # Parse AI response for diagnostics
            # This is a simplified implementation
            diagnostics = []
            
            # For now, return empty diagnostics
            # In a full implementation, you would parse the AI response
            # and create Diagnostic objects
            
            return diagnostics
            
        except Exception as e:
            logger.error(f"Error getting diagnostics: {e}")
            return []
    
    async def _get_code_actions(self, content: str, range_param: Range, context) -> List[CodeAction]:
        """Get available code actions."""
        actions = []
        
        # Add some basic code actions
        actions.append(
            CodeAction(
                title="Format Document",
                kind=CodeActionKind.SourceFormatDocument,
                command={
                    "title": "Format Document",
                    "command": "asthra.format.document"
                }
            )
        )
        
        return actions
    
    def _apply_incremental_change(self, doc_uri: str, change: TextDocumentContentChangeEvent):
        """Apply incremental document change."""
        # Simplified implementation
        # In a full implementation, you would apply the range-based change
        pass
    
    def start_server(self, host: str = "127.0.0.1", port: int = 9000):
        """Start the LSP server."""
        logger.info(f"Starting Asthra LSP server on {host}:{port}")
        logger.info(f"Model API: {self.model_api_base}")
        
        self.server.start_tcp(host, port)

def main():
    """Main function to start the LSP server."""
    import argparse
    
    parser = argparse.ArgumentParser(description="Asthra Language Server Protocol Server")
    parser.add_argument('--host', default='127.0.0.1', help='Server host')
    parser.add_argument('--port', type=int, default=9000, help='Server port')
    parser.add_argument('--model-api', default='http://localhost:8000/v1', help='Model API base URL')
    parser.add_argument('--verbose', action='store_true', help='Verbose logging')
    
    args = parser.parse_args()
    
    if args.verbose:
        logging.getLogger().setLevel(logging.DEBUG)
    
    print("🚀 Starting Asthra Language Server Protocol Server")
    print("=" * 50)
    print(f"Host: {args.host}")
    print(f"Port: {args.port}")
    print(f"Model API: {args.model_api}")
    print()
    print("Configure your editor to connect to this LSP server:")
    print(f"  TCP: {args.host}:{args.port}")
    print()
    
    try:
        lsp_server = AsthraiLanguageServer(args.model_api)
        lsp_server.start_server(args.host, args.port)
    except KeyboardInterrupt:
        print("\n🛑 Server stopped by user")
    except Exception as e:
        print(f"❌ Server error: {e}")
        sys.exit(1)

if __name__ == '__main__':
    main() 
