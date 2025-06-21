use ampu;
use std::error::Error;
use std::process;

#[tokio::main]
async fn main() {
    if let Err(e) = ampu::run().await {
        eprintln!("Error: {}", e);
        
        // Print the error chain for better debugging
        let mut source = e.source();
        while let Some(err) = source {
            eprintln!("  Caused by: {}", err);
            source = err.source();
        }
        
        process::exit(1);
    }
} 
