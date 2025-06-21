pub mod build;
pub mod check;
pub mod fmt;
pub mod run;
pub mod test;
pub mod mod_commands;

use clap::{Parser, Subcommand};
use crate::error::Result;

#[derive(Parser)]
#[command(name = "ampu")]
#[command(about = "Asthra build tool for dependency management and compilation orchestration")]
#[command(version = "0.1.0")]
pub struct Cli {
    #[command(subcommand)]
    pub command: Commands,
    
    #[arg(long, global = true)]
    pub verbose: bool,
    
    #[arg(long, global = true)]
    pub quiet: bool,
}

#[derive(Subcommand)]
pub enum Commands {
    /// Build the current package
    Build {
        #[arg(long)]
        release: bool,
        
        #[arg(long)]
        target: Option<String>,
        
        #[arg(short, long)]
        jobs: Option<usize>,
    },
    
    /// Fast check files for errors and warnings
    FastCheck(check::CheckCommand),
    
    /// Format source code
    Fmt {
        /// Check if files are formatted without writing changes
        #[arg(long)]
        check: bool,
        
        /// Show diff of changes without writing
        #[arg(long)]
        diff: bool,
        
        /// Exclude files matching the given glob pattern
        #[arg(long)]
        exclude: Vec<String>,
        
        /// Only format files matching the given glob pattern
        #[arg(long)]
        include: Vec<String>,
        
        /// Files or directories to format
        #[arg(value_name = "PATH")]
        paths: Vec<String>,
    },
    
    /// Build and run the current package
    Run {
        #[arg(long)]
        release: bool,
        
        #[arg(last = true)]
        args: Vec<String>,
    },
    
    /// Run tests
    Test {
        #[arg(long)]
        release: bool,
        
        #[arg(long)]
        no_capture: bool,
        
        #[arg(value_name = "TESTNAME")]
        test_name: Option<String>,
    },
    
    /// Initialize a new module
    Init {
        #[arg(value_name = "NAME")]
        name: Option<String>,
        
        #[arg(long)]
        lib: bool,
    },
    
    /// Add a dependency
    Add {
        #[arg(value_name = "PACKAGE")]
        package: String,
        
        #[arg(long)]
        dev: bool,
        
        #[arg(long)]
        version: Option<String>,
    },
    
    /// Remove a dependency
    Remove {
        #[arg(value_name = "PACKAGE")]
        package: String,
        
        #[arg(long)]
        dev: bool,
    },
    
    /// Update dependencies
    Update {
        #[arg(value_name = "PACKAGE")]
        package: Option<String>,
    },
    
    /// List dependencies
    List {
        #[arg(long)]
        tree: bool,
    },
    
    /// Clean build artifacts
    Clean {
        #[arg(long)]
        all: bool,
        
        #[arg(long)]
        profile: Option<String>,
        
        #[arg(long)]
        deps: bool,
        
        #[arg(long)]
        cache: bool,
    },
    
    /// Check imports and dependencies without building
    Check {
        #[arg(long)]
        access_control: bool,
    },
    
    /// Show dependency information
    Deps {
        #[arg(long)]
        tree: bool,
        
        #[arg(long)]
        graph: bool,
        
        #[arg(long)]
        format: Option<String>,
    },
    
    /// Vendor dependencies locally
    Vendor,
    
    /// Explain why a package is included
    Why {
        #[arg(value_name = "PACKAGE")]
        package: String,
    },
    
    /// Generate dependency graph
    Graph {
        #[arg(long, default_value = "text")]
        format: String,
        
        #[arg(short, long)]
        output: Option<String>,
    },
    
    /// Generate documentation
    Doc {
        #[arg(long)]
        open: bool,
        
        #[arg(long)]
        format: Option<String>,
        
        #[arg(long)]
        serve: bool,
        
        #[arg(long, default_value = "8080")]
        port: u16,
    },
}

pub async fn run_cli() -> Result<()> {
    let cli = Cli::parse();
    
    // Set up logging based on verbosity
    setup_logging(cli.verbose, cli.quiet)?;
    
    match cli.command {
        Commands::Build { release, target, jobs } => {
            build::handle_build(release, target, jobs).await
        }
        Commands::FastCheck(cmd) => {
            cmd.execute().await.map_err(|e| crate::error::BuildError::Compiler(e.to_string()))
        }
        Commands::Fmt { check, diff, exclude, include, paths } => {
            fmt::handle_fmt(check, diff, exclude, include, paths).await
        }
        Commands::Run { release, args } => {
            run::handle_run(release, args).await
        }
        Commands::Test { release, no_capture, test_name } => {
            test::handle_test(release, no_capture, test_name).await
        }
        Commands::Init { name, lib } => {
            mod_commands::handle_init(name, lib).await
        }
        Commands::Add { package, dev, version } => {
            mod_commands::handle_add(package, dev, version).await
        }
        Commands::Remove { package, dev } => {
            mod_commands::handle_remove(package, dev).await
        }
        Commands::Update { package } => {
            mod_commands::handle_update(package).await
        }
        Commands::List { tree } => {
            mod_commands::handle_list(tree).await
        }
        Commands::Clean { all, profile, deps, cache } => {
            mod_commands::handle_clean(all, profile, deps, cache).await
        }
        Commands::Check { access_control } => {
            mod_commands::handle_check(access_control).await
        }
        Commands::Deps { tree, graph, format } => {
            mod_commands::handle_deps(tree, graph, format).await
        }
        Commands::Vendor => {
            mod_commands::handle_vendor().await
        }
        Commands::Why { package } => {
            mod_commands::handle_why(package).await
        }
        Commands::Graph { format, output } => {
            mod_commands::handle_graph(format, output).await
        }
        Commands::Doc { open, format, serve, port } => {
            mod_commands::handle_doc(open, format, serve, port).await
        }
    }
}

fn setup_logging(verbose: bool, quiet: bool) -> Result<()> {
    let log_level = if quiet {
        log::LevelFilter::Error
    } else if verbose {
        log::LevelFilter::Debug
    } else {
        log::LevelFilter::Info
    };
    
    env_logger::Builder::from_default_env()
        .filter_level(log_level)
        .init();
    
    Ok(())
} 
