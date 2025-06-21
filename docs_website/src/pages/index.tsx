import type {ReactNode} from 'react';
import clsx from 'clsx';
import Link from '@docusaurus/Link';
import useDocusaurusContext from '@docusaurus/useDocusaurusContext';
import Layout from '@theme/Layout';
import HomepageFeatures from '@site/src/components/HomepageFeatures';
import Heading from '@theme/Heading';

import styles from './index.module.css';

function HomepageHeader() {
  const {siteConfig} = useDocusaurusContext();
  return (
    <header className={clsx('hero hero--primary', styles.heroBanner)}>
      <div className="container">
        <Heading as="h1" className="hero__title">
          {siteConfig.title}
        </Heading>
        <p className="hero__subtitle">{siteConfig.tagline}</p>
        <div className={styles.buttons}>
          <Link
            className="button button--secondary button--lg"
            to="/user-manual/getting-started">
            Get Started 🚀
          </Link>
          <Link
            className="button button--outline button--secondary button--lg"
            to="/spec/overview">
            Language Overview 📖
          </Link>
        </div>
      </div>
    </header>
  );
}

function HomepageStats() {
  return (
    <section className={styles.stats}>
      <div className="container">
        <div className="row">
          <div className="col col--12">
            <div className="text--center">
              <Heading as="h2">Production Ready Status</Heading>
              <p className="hero__subtitle">
                Asthra has achieved production-ready status with comprehensive testing and validation
              </p>
            </div>
          </div>
        </div>
        <div className="row">
          <div className="col col--3">
            <div className="text--center">
              <div className={styles.statNumber}>31</div>
              <div className={styles.statLabel}>Test Categories</div>
              <div className={styles.statSubtext}>100% Success Rate</div>
            </div>
          </div>
          <div className="col col--3">
            <div className="text--center">
              <div className={styles.statNumber}>130+</div>
              <div className={styles.statLabel}>Total Tests</div>
              <div className={styles.statSubtext}>All Passing</div>
            </div>
          </div>
          <div className="col col--3">
            <div className="text--center">
              <div className={styles.statNumber}>v2.0</div>
              <div className={styles.statLabel}>Current Version</div>
              <div className={styles.statSubtext}>Production Ready</div>
            </div>
          </div>
          <div className="col col--3">
            <div className="text--center">
              <div className={styles.statNumber}>10x</div>
              <div className={styles.statLabel}>Faster Development</div>
              <div className={styles.statSubtext}>With AI Integration</div>
            </div>
          </div>
        </div>
      </div>
    </section>
  );
}

function CodeExample() {
  return (
    <section className={styles.codeExample}>
      <div className="container">
        <div className="row">
          <div className="col col--12">
            <div className="text--center margin-bottom--lg">
              <Heading as="h2">AI-Friendly Syntax</Heading>
              <p>
                Clean, predictable patterns that both humans and AI can understand
              </p>
            </div>
          </div>
        </div>
        <div className="row">
          <div className="col col--6">
            <h3>Safe Systems Programming</h3>
            <pre className={styles.codeBlock}>
{`// Safe C interoperability
extern "C" fn malloc(size: usize) -> #[transfer_full] *mut void;

pub fn allocate_buffer(size: usize) -> *mut u8 {
    unsafe {
        let ptr = malloc(size);
        if ptr.is_null() {
            panic("Allocation failed");
        }
        return ptr;
    }
}`}
            </pre>
          </div>
          <div className="col col--6">
            <h3>Concurrency & Error Handling</h3>
            <pre className={styles.codeBlock}>
{`// Built-in concurrency and Result types
pub fn process_data(data: []u8) -> Result<ProcessedData, Error> {
    let result = spawn {
        let processed = transform_data(data);
        return Result.Ok(processed);
    };
    
    return await result;
}`}
            </pre>
          </div>
        </div>
      </div>
    </section>
  );
}

export default function Home(): ReactNode {
  const {siteConfig} = useDocusaurusContext();
  return (
    <Layout
      title={`${siteConfig.title} - AI-First Systems Programming`}
      description="The world's first production-ready programming language designed for AI code generation excellence. Safe, fast, and AI-friendly systems programming.">
      <HomepageHeader />
      <main>
        <HomepageStats />
        <HomepageFeatures />
        <CodeExample />
      </main>
    </Layout>
  );
}
